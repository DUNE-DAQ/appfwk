# Set moo schema search path
from dunedaq.env import get_moo_model_path
import moo.io
moo.io.default_load_path = get_moo_model_path()

from os.path import exists, join
from rich.console import Console
from copy import deepcopy
from collections import namedtuple, defaultdict
import json
import os
from enum import Enum
from graphviz import Digraph
import networkx as nx
import moo.otypes

moo.otypes.load_types('rcif/cmd.jsonnet')
moo.otypes.load_types('appfwk/cmd.jsonnet')
moo.otypes.load_types('appfwk/app.jsonnet')

moo.otypes.load_types('nwqueueadapters/networktoqueue.jsonnet')
moo.otypes.load_types('nwqueueadapters/queuetonetwork.jsonnet')
moo.otypes.load_types('trigger/moduleleveltrigger.jsonnet')

from appfwk.utils import acmd, mcmd, mspec
import dunedaq.nwqueueadapters.networkobjectsender as nos
import dunedaq.nwqueueadapters.queuetonetwork as qton
import dunedaq.nwqueueadapters.networkobjectreceiver as nor
import dunedaq.nwqueueadapters.networktoqueue as ntoq
import dunedaq.appfwk.app as appfwk  # AddressedCmd,
import dunedaq.rcif.cmd as rccmd  # AddressedCmd,
import dunedaq.trigger.moduleleveltrigger as mlt

console = Console()

########################################################################
#
# Classes
#
########################################################################

# TODO: Connections between modules are held in the module object, but
# connections between applications are held in their own
# structure. Not clear yet which is better, but should probably be
# consistent either way

# TODO: Understand whether extra_commands is actually needed. Seems like "resume" is already being sent to everyone?

# TODO: Make these all dataclasses

class Module:
    """An individual DAQModule within an application, along with its
       configuration object and list of outgoing connections to other
       modules
    """

    def __init__(self, plugin, conf=None, connections=None):
        self.plugin=plugin
        self.conf=conf
        self.connections=connections if connections else dict()

    def __repr__(self):
        return f"module(plugin={self.plugin}, conf={self.conf}, connections={self.connections})"

    def __rich_repr__(self):
        yield "plugin", self.plugin
        yield "conf", self.conf
        yield "connections", self.connections


Connection = namedtuple("Connection", ['to', 'queue_kind', 'queue_capacity', 'queue_name', 'toposort'], defaults=("FollyMPMCQueue", 1000, None, True))

class Direction(Enum):
    IN = 1
    OUT = 2

Endpoint = namedtuple("Endpoint", [ 'external_name', 'internal_name', 'direction' ])
GeoID = namedtuple('GeoID', ['system', 'region', 'element'])
FragmentProducer = namedtuple('FragmentProducer', ['geoid', 'requests_in', 'fragments_out', 'queue_name'])

class ModuleGraph:
    """A set of modules and connections between them.

    modulegraph holds a dictionary of modules, with each module
    knowing its (outgoing) connections to other modules in the
    modulegraph.

    Connections to other modulegraphs are represented by
    `endpoints`. The endpoint's `external_name` is the "public" name
    of the connection, which other modulegraphs should use. The
    endpoint's `internal_name` specifies the particular module and
    sink/source name which the endpoint is connected to, which may be
    changed without affecting other applications.

    """
    def __init__(self, modules=None, endpoints=None, fragment_producers=None):
        self.modules=modules if modules else dict()
        self.endpoints=endpoints if endpoints else dict()
        self.fragment_producers = fragment_producers if  fragment_producers else dict()

    def __repr__(self):
        return f"modulegraph(modules={self.modules}, endpoints={self.endpoints}, fragment_producers={self.fragment_producers})"

    def __rich_repr__(self):
        yield "modules", self.modules
        yield "endpoints", self.endpoints
        yield "fragment_producers", self.fragment_producers

    def set_from_dict(self, module_dict):
        self.modules=module_dict

    def module_names(self):
        return self.modules.keys()

    def module_list(self):
        return self.modules.values()

    def add_module(self, name, **kwargs):
        mod=module(**kwargs)
        self.modules[name]=mod
        return mod

    def add_connection(self, from_endpoint, to_endpoint):
        from_mod, from_name=from_endpoint.split(".")
        self.modules[from_mod].connections[from_name]=Connection(to_endpoint)

    def add_endpoint(self, external_name, internal_name, inout):
        self.endpoints[external_name]=Endpoint(external_name, internal_name, inout)

    def endpoint_names(self, inout=None):
        if inout is not None:
            return [ e[0] for e in self.endpoints.items() if e[1].inout==inout ]
        return self.endpoints.keys()

    def add_fragment_producer(self, system, region, element, requests_in, fragments_out):
        geoid = GeoID(system, region, element)
        if geoid in self.fragment_producers:
            raise ValueError(f"There is already a fragment producer for GeoID {geoid}")
        # Can't set queue_name here, because the queue names need to be unique system-wide, but we're inside a particular app here. Instead, we create the queue names in readout_gen.generate, where all of the fragment producers are known
        queue_name = None
        self.fragment_producers[geoid] = FragmentProducer(geoid, requests_in, fragments_out, queue_name)

class App:
    """A single daq_application in a system, consisting of a modulegraph
       and a hostname on which to run"""
    
    def __init__(self, modulegraph=None, host="localhost"):
        self.modulegraph = modulegraph if modulegraph else ModuleGraph()
        self.host = host

Publisher = namedtuple(
    "Publisher", ['msg_type', 'msg_module_name', 'subscribers'])

Sender = namedtuple("Sender", ['msg_type', 'msg_module_name', 'receiver'])

class System:
    """A full DAQ system consisting of multiple applications and the
    connections between them. The `apps` member is a dictionary from
    application name to app object, and the app_connections member is
    a dictionary from upstream endpoint to publisher or sender object
    representing the downstream endpoint(s). Endpoints are specified
    as strings like app_name.endpoint_name.

    An explicit mapping from upstream endpoint name to zeromq
    connection string may be specified, but typical usage is to not
    specify this, and leave the mapping to be automatically generated.

    The same is true for application start order.

    """
    def __init__(self, apps=None, app_connections=None, network_endpoints=None, app_start_order=None):
        self.apps=apps if apps else dict()
        self.app_connections=app_connections if app_connections else dict()
        self.network_endpoints=network_endpoints
        self.app_start_order=app_start_order

    def __rich_repr__(self):
        yield "apps", self.apps
        yield "app_connections", self.app_connections
        yield "network_endpoints", self.network_endpoints
        yield "app_start_order", self.app_start_order

    def get_fragment_producers(self):
        """Get a list of all the fragment producers in the system"""
        all_producers = []
        all_geoids = set()
        for app in self.apps.values():
            console.log(app)
            producers = app.modulegraph.fragment_producers
            for producer in producers.values():
                if producer.geoid in all_geoids:
                    raise ValueError(f"GeoID {producer.geoid} has multiple fragment producers")
                all_geoids.add(producer.geoid)
                all_producers.append(producer)
        return all_producers

########################################################################
#
# Functions
#
########################################################################

def make_module_deps(module_dict):
    """Given a dictionary of `module` objects, produce a dictionary giving
    the dependencies between them. A dependency is any connection between
    modules (implemented using an appfwk queue). Connections whose
    upstream ends begin with a '!' are not considered dependencies, to
    allow us to break cycles in the DAG.

    Returns a networkx DiGraph object where nodes are module names

    """

    deps = nx.DiGraph()
    for mod_name in module_dict.keys():
        deps.add_node(mod_name)
    for name, mod in module_dict.items():
        for upstream_name, downstream_connection in mod.connections.items():
            if downstream_connection.toposort:
                other_mod = downstream_connection.to.split(".")[0]
                deps.add_edge(name, other_mod)
    return deps


def make_app_deps(the_system, verbose=False):
    """Produce a dictionary giving
    the dependencies between a set of applications, given their connections.

    Returns a networkx DiGraph object where nodes are app names

    """

    deps = nx.DiGraph()

    for app in the_system.apps.keys():
        deps.add_node(app)

    for from_endpoint, conn in the_system.app_connections.items():
        from_app = from_endpoint.split(".")[0]
        if hasattr(conn, "subscribers"):
            for to_app in [ds.split(".")[0] for ds in conn.subscribers]:
                deps.add_edge(from_app, to_app)
        elif hasattr(conn, "receiver"):
            to_app = conn.receiver.split(".")[0]
            deps.add_edge(from_app, to_app)

    return deps


def make_app_command_data(app, verbose=False):
    """Given an App instance, create the 'command data' suitable for
    feeding to nanorc. The needed queues are inferred from from
    connections between modules, as are the start and stop order of the
    modules

    TODO: This should probably be split up into separate stages of
    inferring/creating the queues (which can be part of validation)
    and actually making the command data objects for nanorc.

    """

    
    modules = app.modulegraph.modules

    module_deps = make_module_deps(modules)
    if verbose:
        console.log(f"inter-module dependencies are: {module_deps}")

    start_order = list(nx.algorithms.dag.topological_sort(module_deps))
    stop_order = start_order[::-1]

    if verbose:
        console.log(f"Inferred module start order is {start_order}")
        console.log(f"Inferred module stop order is {stop_order}")

    command_data = {}

    queue_specs = []

    app_qinfos = defaultdict(list)

    # Infer the queues we need based on the connections between modules

    # Terminology: an "endpoint" is "module.name"
    for name, mod in modules.items():
        for from_name, downstream_connection in mod.connections.items():
            # The name might be prefixed with a "!" to indicate that it doesn't participate in dependencies. Remove that here because "!" is illegal in actual queue names
            from_name = from_name.replace("!", "")
            from_endpoint = ".".join([name, from_name])
            to_endpoint=downstream_connection.to
            to_mod, to_name = to_endpoint.split(".")
            queue_inst = f"{from_endpoint}_to_{to_endpoint}".replace(".", "")
            # Is there already a queue connecting either endpoint? If so, we reuse it

            # TODO: This is a bit complicated. Might be nicer to find
            # the list of necessary queues in a first step, and then
            # actually make the QueueSpec/QueueInfo objects
            found_from = False
            found_to = False
            for k, v in app_qinfos.items():
                for qi in v:
                    test_endpoint = ".".join([k, qi.name])
                    if test_endpoint == from_endpoint:
                        found_from = True
                        queue_inst = qi.inst
                    if test_endpoint == to_endpoint:
                        found_to = True
                        queue_inst = qi.inst

            if not (found_from or found_to):
                queue_inst = queue_inst if downstream_connection.queue_name is None else downstream_connection.queue_name
                if verbose:
                    console.log(f"Creating {downstream_connection.queue_kind}({downstream_connection.queue_capacity}) queue with name {queue_inst} connecting {from_endpoint} to {to_endpoint}")
                queue_specs.append(appfwk.QueueSpec(
                    inst=queue_inst, kind=downstream_connection.queue_kind, capacity=downstream_connection.queue_capacity))

            if not found_from:
                if verbose:
                    console.log(f"Adding output queue to module {name}: inst={queue_inst}, name={from_name}")
                app_qinfos[name].append(appfwk.QueueInfo(
                    name=from_name, inst=queue_inst, dir="output"))
            if not found_to:
                if verbose:
                    console.log(f"Adding input queue to module {to_mod}: inst={queue_inst}, name={to_name}")
                app_qinfos[to_mod].append(appfwk.QueueInfo(
                    name=to_name, inst=queue_inst, dir="input"))

    mod_specs = [mspec(name, mod.plugin, app_qinfos[name])
                 for name, mod in modules.items()]

    # Fill in the "standard" command entries in the command_data structure

    command_data['init'] = appfwk.Init(queues=queue_specs, modules=mod_specs)

    # TODO: Conf ordering
    command_data['conf'] = acmd([
        (name, mod.conf) for name, mod in modules.items()
    ])

    startpars = rccmd.StartParams(run=1, disable_data_storage=False)

    command_data['start'] = acmd([(name, startpars) for name in start_order])
    command_data['stop'] = acmd([(name, None) for name in stop_order])
    command_data['scrap'] = acmd([(name, None) for name in stop_order])

    # Optional commands

    # TODO: What does an empty "acmd" actually imply? Does the command get sent to everyone, or no-one?
    command_data['pause'] = acmd([])
    command_data['resume'] = acmd([])

    # TODO: handle modules' `extra_commands`, including "record"

    return command_data

def geoid_raw_str(geoid):
    """Get a string representation of a GeoID suitable for using in queue names"""
    return f"geoid{geoid.system}_{geoid.region}_{geoid.element}"

def data_request_endpoint_name(producer):
    return f"data_request_{geoid_raw_str(producer.geoid)}"

def set_mlt_links(the_system, mlt_app_name="trigger", verbose=False):
    """The MLT needs to know the full list of fragment producers in the
system so it can populate the TriggerDecisions it creates. This
function gets all the fragment producers in the system and adds their
GeoIDs to the MLT's config. It assumes that the ModuleLevelTrigger
lives in an application with name `mlt_app_name` and has the name
"mlt"."""
    mlt_links = []
    for producer in the_system.get_fragment_producers():
        geoid = producer.geoid
        mlt_links.append( mlt.GeoID(system=geoid.system, region=geoid.region, element=geoid.element) )
    # Now we add the full set of links to the MLT plugin conf. It
    # would be nice to just modify the `links` attribute of the
    # mlt.ConfParams object, but moo-derived objects work in a funny
    # way (returning a copy of the attribute, not returning a
    # reference to it), which means we have to copy and replace the
    # whole thing
    if verbose:
        console.log(f"Adding {len(mlt_links)} links to mlt.links: {mlt_links}")
    old_mlt = deepcopy(the_system.apps["trigger"].modulegraph.modules["mlt"])
    the_system.apps[mlt_app_name].modulegraph.modules["mlt"] = Module(plugin=old_mlt.plugin,
                                                                      conf=mlt.ConfParams(links=mlt_links,
                                                                                          initial_token_count=old_mlt.conf.initial_token_count),
                                                                      connections=old_mlt.connections)


def connect_fragment_producers(app_name, the_system, verbose=False):
    """Connect the data request and fragment sending queues from all of
       the fragment producers in the app with name `app_name` to the
       appropriate endpoints of the dataflow app."""
    if verbose:
        console.log(f"Connecting fragment producers in {app_name}")

    app = the_system.apps[app_name]
    producers = app.modulegraph.fragment_producers

    for producer in producers.values():
        request_endpoint = data_request_endpoint_name(producer)
        if verbose:
            console.log(f"Creating request endpoint {request_endpoint}")
        app.modulegraph.add_endpoint(request_endpoint, producer.requests_in, Direction.IN)
        the_system.app_connections[f"dataflow.{data_request_endpoint_name(producer)}"] = Sender(msg_type="dunedaq::dfmessages::DataRequest",
                                                                               msg_module_name="DataRequestNQ",
                                                                               receiver=f"{app_name}.{request_endpoint}")

        frag_endpoint = f"fragments_{geoid_raw_str(producer.geoid)}"
        if verbose:
            console.log(f"Creating fragment endpoint {frag_endpoint}")
        app.modulegraph.add_endpoint(frag_endpoint, producer.fragments_out, Direction.OUT)

        the_system.app_connections[f"{app_name}.{frag_endpoint}"] = Sender(msg_type="std::unique_ptr<dunedaq::daqdataformats::Fragment>",
                                                                           msg_module_name="FragmentNQ",
                                                                           receiver=f"dataflow.fragments")

def connect_all_fragment_producers(the_system, dataflow_name="dataflow", verbose=False):
    """Connect all fragment producers in the system to the appropriate
       queues in the dataflow app."""
    for name, app in the_system.apps.items():
        if name==dataflow_name:
            continue
        connect_fragment_producers(name, the_system, verbose)
            
def assign_network_endpoints(the_system, verbose=False):
    """Given a set of applications and connections between them, come up
    with a list of suitable zeromq endpoints. Return value is a mapping
    from name of upstream end of connection to endpoint name.

    Algorithm is to make an endpoint name like tcp://host:port, where
    host is the hostname for the app at the upstream end of the
    connection, port starts at some fixed value, and increases by 1
    for each new endpoint.

    You might think that we could reuse port numbers for different
    apps, but that's not possible since multiple applications may run
    on the same host, and we don't know the _actual_ host here, just,
    eg "{host_dataflow}", which is later interpreted by nanorc.

    """
    endpoints = {}
    #host_ports = defaultdict(int)
    port = 12345
    for conn in the_system.app_connections.keys():
        app = conn.split(".")[0]
        #host = the_system.apps[app].host
        # if host == "localhost":
        #     host = "127.0.0.1"
        #port = first_port + host_ports[host]
        #host_ports[host] += 1
        endpoints[conn] = f"tcp://{{host_{app}}}:{port}"
        if verbose:
            console.log(f"Assigned endpoint {endpoints[conn]} for connection {conn}")
        port+=1
    return endpoints


def resolve_endpoint(app, external_name, inout, verbose=False):
    """Resolve an `external` endpoint name to the corresponding internal "module.sinksource"
    """
    if external_name in app.modulegraph.endpoints:
        e=app.modulegraph.endpoints[external_name]
        if e.direction==inout:
            if verbose:
                console.log(f"Endpoint {external_name} resolves to {e.internal_name}")
            return e.internal_name
        else:
            raise KeyError(f"Endpoint {external_name} has direction {e.direction}, but requested direction was {inout}")
    else:
        raise KeyError(f"Endpoint {external_name} not found")

def make_unique_name(base, dictionary):
    suffix=0
    while f"{base}{suffix}" in dictionary:
        suffix+=1
    assert f"{base}{suffix}" not in dictionary

    return f"{base}{suffix}"

def add_network(app_name, the_system, verbose=False):
    """Add the necessary QueueToNetwork and NetworkToQueue objects to the
       application named `app_name`, based on the inter-application
       connections specified in `the_system`. NB `the_system` is modified
       in-place."""

    if the_system.network_endpoints is None:
        the_system.network_endpoints=assign_network_endpoints(the_system)

    app = the_system.apps[app_name]

    modules_with_network = deepcopy(app.modulegraph.modules)

    unconnected_endpoints = set(app.modulegraph.endpoints.keys())

    if verbose:
        console.log(f"Endpoints to connect are: {unconnected_endpoints}")

    for conn_name, conn in the_system.app_connections.items():
        from_app, from_endpoint = conn_name.split(".", maxsplit=1)

        if from_app == app_name:
            unconnected_endpoints.remove(from_endpoint)
            from_endpoint = resolve_endpoint(app, from_endpoint, Direction.OUT)
            from_endpoint_module, from_endpoint_sink = from_endpoint.split(".")
            # We're a publisher or sender. Make the queue to network
            qton_name = conn_name.replace(".", "_")
            qton_name = make_unique_name(qton_name, modules_with_network)

            if verbose:
                console.log(f"Adding QueueToNetwork named {qton_name} connected to {from_endpoint} in app {app_name}")
            modules_with_network[qton_name] = Module(plugin="QueueToNetwork",
                                                     connections={}, # No outgoing connections
                                                     conf=qton.Conf(msg_type=conn.msg_type,
                                                                    msg_module_name=conn.msg_module_name,
                                                                    sender_config=nos.Conf(ipm_plugin_type="ZmqPublisher" if type(conn) == Publisher else "ZmqSender",
                                                                                           address=the_system.network_endpoints[conn_name],
                                                                                           topic="foo",
                                                                                           stype="msgpack")))
            # Connect the module to the QueueToNetwork
            mod_connections = modules_with_network[from_endpoint_module].connections
            mod_connections[from_endpoint_sink] = Connection(f"{qton_name}.input")

        if hasattr(conn, "subscribers"):
            for to_conn in conn.subscribers:
                to_app, to_endpoint = to_conn.split(".", maxsplit=1)

                if app_name == to_app:
                    if verbose:
                        console.log(f"App {app_name} endpoint {to_endpoint} is being connected")

                    # For pub/sub connections, we might connect
                    # multiple times to the same endpoint, so it might
                    # already have been removed from the list
                    if to_endpoint in unconnected_endpoints:
                        unconnected_endpoints.remove(to_endpoint)
                    to_endpoint = resolve_endpoint(app, to_endpoint, Direction.IN)
                    ntoq_name = to_conn.replace(".", "_")
                    ntoq_name = make_unique_name(ntoq_name, modules_with_network)

                    if verbose:
                        console.log(f"Adding NetworkToQueue named {ntoq_name} connected to {to_endpoint} in app {app_name}")

                    modules_with_network[ntoq_name] = Module(plugin="NetworkToQueue",
                                                             connections={
                                                                 "output": Connection(to_endpoint)},
                                                             conf=ntoq.Conf(msg_type=conn.msg_type,
                                                                            msg_module_name=conn.msg_module_name,
                                                                            receiver_config=nor.Conf(ipm_plugin_type="ZmqSubscriber",
                                                                                                     address=the_system.network_endpoints[
                                                                                                         conn_name],
                                                                                                     subscriptions=["foo"])))

        if hasattr(conn, "receiver") and app_name == conn.receiver.split(".")[0]:
            # We're a receiver. Add a NetworkToQueue of receiver type
            #
            # TODO: DRY
            to_app, to_endpoint = conn.receiver.split(".", maxsplit=1)
            if to_endpoint in unconnected_endpoints:
                unconnected_endpoints.remove(to_endpoint)
            to_endpoint = resolve_endpoint(app, to_endpoint, Direction.IN)

            ntoq_name = conn.receiver.replace(".", "_")
            ntoq_name = make_unique_name(ntoq_name, modules_with_network)

            if verbose:
                console.log(f"Adding NetworkToQueue named {ntoq_name} connected to {to_endpoint} in app {app_name}")
            modules_with_network[ntoq_name] = Module(plugin="NetworkToQueue",
                                                     connections={
                                                         "output": Connection(to_endpoint)},
                                                     conf=ntoq.Conf(msg_type=conn.msg_type,
                                                                    msg_module_name=conn.msg_module_name,
                                                                    receiver_config=nor.Conf(ipm_plugin_type="ZmqReceiver",
                                                                                             address=the_system.network_endpoints[conn_name]))
                                                     )

    if unconnected_endpoints:
        # TODO: Use proper logging
        console.log(f"Warning: the following endpoints of {app_name} were not connected to anything: {unconnected_endpoints}")

    app.modulegraph.modules = modules_with_network


def generate_boot(apps: list, partition_name="${USER}_test", ers_settings=None, info_svc_uri="file://info_${APP_ID}_${APP_PORT}.json",
                  disable_trace=False, use_kafka=False, verbose=False) -> dict:
    """Generate the dictionary that will become the boot.json file"""

    if ers_settings is None:
        ers_settings={
            "INFO":    "erstrace,throttle,lstdout",
            "WARNING": "erstrace,throttle,lstdout",
            "ERROR":   "erstrace,throttle,lstdout",
            "FATAL":   "erstrace,lstdout",
        }

    daq_app_specs = {
        "daq_application_ups" : {
            "comment": "Application profile based on a full dbt runtime environment",
            "env": {
                "DBT_AREA_ROOT": "getenv",
                "TRACE_FILE": "getenv:/tmp/trace_buffer_${HOSTNAME}_${USER}",
            },
            "cmd": ["CMD_FAC=rest://localhost:${APP_PORT}",
                    "INFO_SVC=" + info_svc_uri,
                    "cd ${DBT_AREA_ROOT}",
                    "source dbt-env.sh",
                    "dbt-workarea-env",
                    "cd ${APP_WD}",
                    "daq_application --name ${APP_NAME} -c ${CMD_FAC} -i ${INFO_SVC}"]
        },
        "daq_application" : {
            "comment": "Application profile using  PATH variables (lower start time)",
            "env":{
                "CET_PLUGIN_PATH": "getenv",
                "DUNEDAQ_SHARE_PATH": "getenv",
                "TIMING_SHARE": "getenv",
                "LD_LIBRARY_PATH": "getenv",
                "PATH": "getenv",
                "READOUT_SHARE": "getenv",
                "TRACE_FILE": "getenv:/tmp/trace_buffer_${HOSTNAME}_${USER}",
            },
            "cmd": ["CMD_FAC=rest://localhost:${APP_PORT}",
                    "INFO_SVC=" + info_svc_uri,
                    "cd ${APP_WD}",
                    "daq_application --name ${APP_NAME} -c ${CMD_FAC} -i ${INFO_SVC}"]
        }
    }

    first_port = 3333
    ports = {}
    for i, name in enumerate(apps.keys()):
        ports[name] = first_port + i

    boot = {
        "env": {
            "DUNEDAQ_ERS_VERBOSITY_LEVEL": "getenv:1",
            "DUNEDAQ_PARTITION": partition_name,
            "DUNEDAQ_ERS_INFO": ers_settings["INFO"],
            "DUNEDAQ_ERS_WARNING": ers_settings["WARNING"],
            "DUNEDAQ_ERS_ERROR": ers_settings["ERROR"],
            "DUNEDAQ_ERS_FATAL": ers_settings["FATAL"],
            "DUNEDAQ_ERS_DEBUG_LEVEL": "getenv:-1",
        },
        "apps": {
            name: {
                "exec": "daq_application",
                "host": f"host_{name}",
                "port": ports[name]
            }
            for name, app in apps.items()
        },
        "hosts": {
            f"host_{name}": app.host
            for name, app in apps.items()
        },
        "response_listener": {
            "port": 56789
        },
        "exec": daq_app_specs
    }

    if disable_trace:
        del boot["exec"]["daq_application"]["env"]["TRACE_FILE"]
        del boot["exec"]["daq_application_ups"]["env"]["TRACE_FILE"]

    if use_kafka:
        boot["env"]["DUNEDAQ_ERS_STREAM_LIBS"] = "erskafka"

    if verbose:
        console.log("Boot data")
        console.log(boot)

    return boot


cmd_set = ["init", "conf", "start", "stop", "pause", "resume", "scrap"]


def make_app_json(app_name, app_command_data, data_dir, verbose=False):
    """Make the json files for a single application"""
    for c in cmd_set:
        with open(f'{join(data_dir, app_name)}_{c}.json', 'w') as f:
            json.dump(app_command_data[c].pod(), f, indent=4, sort_keys=True)

def make_system_command_datas(the_system, verbose=False):
    """Generate the dictionary of commands and their data for the entire system"""

    if the_system.app_start_order is None:
        app_deps = make_app_deps(the_system, verbose)
        the_system.app_start_order = list(nx.algorithms.dag.topological_sort(app_deps))

    system_command_datas=dict()

    for c in cmd_set:
        console.log(f"Generating system {c} command")
        cfg = {
            "apps": {app_name: f'data/{app_name}_{c}' for app_name in the_system.apps.keys()}
        }
        if c == 'start':
            cfg['order'] = the_system.app_start_order
        elif c == 'stop':
            cfg['order'] = the_system.app_start_order[::-1]

        system_command_datas[c]=cfg

        if verbose:
            console.log(cfg)

    console.log(f"Generating boot json file")
    system_command_datas['boot'] = generate_boot(the_system.apps, verbose)

    return system_command_datas

def write_json_files(app_command_datas, system_command_datas, json_dir, verbose=False):
    """Write the per-application and whole-system command data as json files in `json_dir`
    """

    console.rule("JSON file creation")

    if exists(json_dir):
        raise RuntimeError(f"Directory {json_dir} already exists")

    data_dir = join(json_dir, 'data')
    os.makedirs(data_dir)

    # Apps
    for app_name, command_data in app_command_datas.items():
        make_app_json(app_name, command_data, data_dir, verbose)

    # System commands
    for cmd, cfg in system_command_datas.items():
        with open(join(json_dir, f'{cmd}.json'), 'w') as f:
            json.dump(cfg, f, indent=4, sort_keys=True)

    console.log(f"System configuration generated in directory '{json_dir}'")


def make_apps_json(the_system, json_dir, verbose=False):
    """Make the json files for all of the applications"""

    if verbose:
        console.log(f"Input applications:")
        console.log(the_system.apps)

    # ==================================================================
    # Application-level generation

    app_command_datas = dict()

    for app_name, app in the_system.apps.items():
        console.rule(f"Application generation for {app_name}")
        # Add the endpoints and connections that are needed for fragment producers
        #
        # NB: modifies app's modulegraph in-place
        connect_fragment_producers(app_name, the_system, verbose)
        # Add the NetworkToQueue/QueueToNetwork modules that are needed.
        #
        # NB: modifies app's modulegraph in-place
        add_network(app_name, the_system, verbose)

        app_command_datas[app_name] = make_app_command_data(app, verbose)
        if verbose:
            console.log(app_command_datas[app_name])

    # ==================================================================
    # System-level generation

    console.rule("System generation")

    system_command_datas=make_system_command_datas(the_system, verbose)

    # ==================================================================
    # JSON file creation

    write_json_files(app_command_datas, system_command_datas, json_dir, verbose)
