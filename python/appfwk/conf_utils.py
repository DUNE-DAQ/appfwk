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
moo.otypes.load_types('networkmanager/nwmgr.jsonnet')
moo.otypes.load_types('dfmodules/fragmentreceiver.jsonnet')
moo.otypes.load_types('dfmodules/requestreceiver.jsonnet')
moo.otypes.load_types('dfmodules/triggerrecordbuilder.jsonnet')

from appfwk.utils import acmd, mcmd, mspec
import dunedaq.nwqueueadapters.networkobjectsender as nos
import dunedaq.nwqueueadapters.queuetonetwork as qton
import dunedaq.nwqueueadapters.networkobjectreceiver as nor
import dunedaq.nwqueueadapters.networktoqueue as ntoq
import dunedaq.appfwk.app as appfwk  # AddressedCmd,
import dunedaq.rcif.cmd as rccmd  # AddressedCmd,
import dunedaq.trigger.moduleleveltrigger as mlt
import dunedaq.networkmanager.nwmgr as nwmgr
import dunedaq.dfmodules.fragmentreceiver as frcv
import dunedaq.dfmodules.requestreceiver as rrcv
import dunedaq.dfmodules.triggerrecordbuilder as trb

from appfwk.daqmodule import DAQModule

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



Connection = namedtuple("Connection", ['to', 'queue_kind', 'queue_capacity', 'queue_name', 'toposort'], defaults=("FollyMPMCQueue", 1000, None, True))

class Direction(Enum):
    IN = 1
    OUT = 2

class Endpoint:
    # def __init__(self, **kwargs):
    #     if kwargs['connection']:
    #         self.__init_with_nwmgr(**kwargs)
    #     else:
    #         self.__init_with_external_name(**kwargs)
    def __init__(self, external_name, internal_name, direction, topic=[]):
        self.external_name = external_name
        self.internal_name = internal_name
        self.direction = direction
        self.topic = topic

    def __repr__(self):
        return f"{self.external_name}/{self.internal_name}"
    # def __init_with_nwmgr(self, connection, internal_name):
    #     self.nwmgr_connection = connection
    #     self.internal_name = internal_name
    #     self.external_name = None
    #     self.direction = Direction.IN

GeoID = namedtuple('GeoID', ['system', 'region', 'element'])
FragmentProducer = namedtuple('FragmentProducer', ['geoid', 'requests_in', 'fragments_out', 'queue_name'])


Publisher = namedtuple(
    "Publisher", ['msg_type', 'msg_module_name', 'subscribers'])

Sender = namedtuple("Sender", ['msg_type', 'msg_module_name', 'receiver'])

AppConnection = namedtuple("AppConnection", ['nwmgr_connection', 'receivers', 'topics', 'msg_type', 'msg_module_name', 'use_nwqa'], defaults=[None, None, True])

########################################################################
#
# Functions
#
########################################################################

def make_module_deps(modules):
    """
    Given a list of `module` objects, produce a dictionary giving
    the dependencies between them. A dependency is any connection between
    modules (implemented using an appfwk queue). Connections whose
    upstream ends begin with a '!' are not considered dependencies, to
    allow us to break cycles in the DAG.

    Returns a networkx DiGraph object where nodes are module names
    """

    deps = nx.DiGraph()
    for module in modules:
        deps.add_node(module.name)

    # console.log("make_module_deps()")
    for module in modules:
        # console.log(f"{module.name}")
        for upstream_name, downstream_connection in module.connections.items():
            if downstream_connection.toposort and downstream_connection.to is not None:
                other_mod = downstream_connection.to.split(".")[0]
                deps.add_edge(module.name, other_mod)

    return deps


def make_app_deps(the_system, verbose=False):
    """
    Produce a dictionary giving
    the dependencies between a set of applications, given their connections.

    Returns a networkx DiGraph object where nodes are app names
    """

    deps = nx.DiGraph()

    for app in the_system.apps.keys():
        deps.add_node(app)

    if verbose: console.log("make_apps_deps()")
    for from_endpoint, conn in the_system.app_connections.items():
        from_app = from_endpoint.split(".")[0]
        if hasattr(conn, "subscribers"):
            for to_app in [ds.split(".")[0] for ds in conn.subscribers]:
                if verbose: console.log(f"subscribers: {from_app}, {to_app}")
                deps.add_edge(from_app, to_app)
        elif hasattr(conn, "receiver"):
            to_app = conn.receiver.split(".")[0]
            if verbose: console.log(f"receiver: {from_app}, {to_app}")
            deps.add_edge(from_app, to_app)

    return deps

def add_one_command_data(command_data, command, default_params, app, module_order):
    """Add the command data for one command in one app to the command_data object. The modules to be sent the command are listed in `module_order`. If the module has an entry in its extra_commands dictionary for this command, then that entry is used as the parameters to pass to the command, otherwise the `default_params` object is passed"""
    mod_and_params=[]
    for module in module_order:
        extra_commands = app.modulegraph.get_module(module).extra_commands
        if command in extra_commands:
            mod_and_params.append((module, extra_commands[command]))
        else:
            mod_and_params.append((module, default_params))

    command_data[command] = acmd(mod_and_params)
    
def make_app_command_data(system, app, verbose=False):
    """Given an App instance, create the 'command data' suitable for
    feeding to nanorc. The needed queues are inferred from from
    connections between modules, as are the start and stop order of the
    modules

    TODO: This should probably be split up into separate stages of
    inferring/creating the queues (which can be part of validation)
    and actually making the command data objects for nanorc.

    """

    if verbose:
        console.log(f"Making app command data for {app.name}")
        
    modules = app.modulegraph.modules

    module_deps = make_module_deps(modules)
    if verbose:
        console.log(f"inter-module dependencies are: {module_deps}")

    stop_order = list(nx.algorithms.dag.topological_sort(module_deps))
    start_order = stop_order[::-1]

    if verbose:
        console.log(f"Inferred module start order is {start_order}")
        console.log(f"Inferred module stop order is {stop_order}")

    command_data = {}

    queue_specs = []

    app_qinfos = defaultdict(list)

    # Infer the queues we need based on the connections between modules

    # Terminology: an "endpoint" is "module.name"
    for mod in modules:
        name = mod.name
        for from_name, downstream_connection in mod.connections.items():
            # The name might be prefixed with a "!" to indicate that it doesn't participate in dependencies. Remove that here because "!" is illegal in actual queue names
            from_name = from_name.replace("!", "")
            from_endpoint = ".".join([name, from_name])
            to_endpoint=downstream_connection.to
            if to_endpoint is None:
                continue
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
                    console.log(f"downstream_connection is {downstream_connection}, its queue_name is {downstream_connection.queue_name}")
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

    if verbose:
        console.log(f"Creating mod_specs for {[ (mod.name, mod.plugin) for mod in modules ]}")
    mod_specs = [ mspec(mod.name, mod.plugin, app_qinfos[mod.name]) for mod in modules ]

    # Fill in the "standard" command entries in the command_data structure

    command_data['init'] = appfwk.Init(queues=queue_specs, modules=mod_specs, nwconnections=system.network_endpoints)

    # TODO: Conf ordering
    command_data['conf'] = acmd([
        (mod.name, mod.conf) for mod in modules
    ])

    startpars = rccmd.StartParams(run=1, disable_data_storage=False)
    resumepars = rccmd.ResumeParams()
    
    add_one_command_data(command_data, "start",   startpars,  app, start_order)
    add_one_command_data(command_data, "stop",    None,       app, stop_order)
    add_one_command_data(command_data, "scrap",   None,       app, stop_order)
    add_one_command_data(command_data, "resume",  resumepars, app, start_order)
    add_one_command_data(command_data, "pause",   None,       app, stop_order)

    # TODO: handle modules' `extra_commands`, including "record"

    return command_data

def geoid_raw_str(geoid):
    """Get a string representation of a GeoID suitable for using in queue names"""
    return f"geoid{geoid.system}_{geoid.region}_{geoid.element}"

def data_request_endpoint_name(producer):
    return f"data_request_{geoid_raw_str(producer.geoid)}"

def set_mlt_links(the_system, mlt_app_name="trigger", verbose=False):
    """
    The MLT needs to know the full list of fragment producers in the
    system so it can populate the TriggerDecisions it creates. This
    function gets all the fragment producers in the system and adds their
    GeoIDs to the MLT's config. It assumes that the ModuleLevelTrigger
    lives in an application with name `mlt_app_name` and has the name
    "mlt".
    """
    mlt_links = []
    for producer in the_system.get_fragment_producers():
        geoid = producer.geoid
        mlt_links.append( mlt.GeoID(system=geoid.system, region=geoid.region, element=geoid.element) )
    if verbose:
        console.log(f"Adding {len(mlt_links)} links to mlt.links: {mlt_links}")
    mgraph = the_system.apps[mlt_app_name].modulegraph
    old_mlt_conf = mgraph.get_module("mlt").conf
    mgraph.reset_module_conf("mlt", mlt.ConfParams(links=mlt_links))

def connect_fragment_producers(app_name, the_system, verbose=False):
    """Connect the data request and fragment sending queues from all of
       the fragment producers in the app with name `app_name` to the
       appropriate endpoints of the dataflow app."""
    if verbose:
        console.log(f"Connecting fragment producers in {app_name}")

    app = the_system.apps[app_name]
    producers = app.modulegraph.fragment_producers

    # Nothing to do if there are no fragment producers. Return now so we don't create unneeded RequestReceiver and FragmentSender
    if len(producers) == 0:
        return
    
    # Create fragment sender. We can do this before looping over the
    # producers because it doesn't need any settings from them
    app.modulegraph.add_module("fragment_sender",
                               plugin = "FragmentSender",
                               conf = None)

    # For each producer, we:
    # 1. Add it to the GeoID -> queue name map that is used in RequestReceiver
    # 2. Connect the relevant RequestReceiver output queue to the request input queue of the fragment producer
    # 3. Connect the fragment output queue of the producer module to the FragmentSender

    request_connection_name = f"{the_system.partition_name}.data_requests_for_{app_name}"
    
    geoid_to_queue_inst = []
    trb_geoid_to_connection = []
    request_receiver_connections = {}
    for producer in producers.values():
        geoid = producer.geoid
        queue_inst = f"data_request_q_for_{geoid_raw_str(producer.geoid)}"
        geoid_to_queue_inst.append(rrcv.geoidinst(region  = geoid.region,
                                                  element = geoid.element,
                                                  system  = geoid.system,
                                                  queueinstance = queue_inst))
        trb_geoid_to_connection.append(trb.geoidinst(region  = geoid.region,
                                                     element = geoid.element,
                                                     system  = geoid.system,
                                                     connection_name = request_connection_name))
        # It looks like RequestReceiver wants its endpoint names to
        # start "data_request_" for the purposes of checking the queue
        # type, but doesn't care what the queue instance name is (as
        # long as it matches what's in the map above), so we just set
        # the endpoint name and queue instance name to the same thing
        request_receiver_connections[queue_inst] = Connection(producer.requests_in,
                                                              queue_name = queue_inst)
        producer_mod_name, producer_endpoint_name = producer.requests_in.split(".", maxsplit=1)
        # Connect the fragment output queue to the fragment sender
        app.modulegraph.add_connection(producer.fragments_out, "fragment_sender.input_queue")
        

    the_system.network_endpoints.append(nwmgr.Connection(name=request_connection_name, topics=[], address=f"tcp://{{host_{app_name}}}:{the_system.next_unassigned_port()}"))
    
    # Create request receiver

    if verbose:
        console.log(f"Creating request_receiver for {app_name} with geoid_to_queue_inst: {geoid_to_queue_inst}, request_receiver_connections: {request_receiver_connections}")
    app.modulegraph.add_module("request_receiver",
                               plugin = "RequestReceiver",
                               conf = rrcv.ConfParams(map = geoid_to_queue_inst,
                                                      connection_name = request_connection_name),
                               connections = request_receiver_connections)
    
    # Connect request receiver to TRB output in DF app
    request_endpoint_name = f"dataflow.data_requests_for_{app_name}"
    app.modulegraph.add_endpoint("data_requests_in",
                                 internal_name = None, # Agh, request receiver uses nwmgr, so no internal endpoint to connect to
                                 inout = Direction.IN)
    the_system.app_connections[request_endpoint_name] = AppConnection(nwmgr_connection = request_connection_name,
                                                                      receivers = [f"{app_name}.data_requests_in"],
                                                                      topics = [],
                                                                      use_nwqa = False)

    df_apps = [ (name,app) for (name,app) in the_system.apps.items() if name.startswith("dataflow") ]
    # Connect fragment sender output to TRB in DF app (via FragmentReceiver)
    fragment_endpoint_name = "{app_name}.fragments"

    for df_name, df_app in df_apps:

        fragment_connection_name = f"{the_system.partition_name}.fragments_to_{df_name}"
        if not the_system.has_network_endpoint(fragment_connection_name):
            the_system.network_endpoints.append(nwmgr.Connection(name=fragment_connection_name, topics=[], address=f"tcp://{{host_{df_name}}}:{the_system.next_unassigned_port()}"))

        df_mgraph = df_app.modulegraph
        if df_mgraph.get_module("fragment_receiver") is None:
            df_mgraph.add_module("fragment_receiver",
                                 plugin = "FragmentReceiver",
                                 conf = frcv.ConfParams(connection_name=fragment_connection_name))
            df_mgraph.add_endpoint("fragments", None,    Direction.IN)
            df_mgraph.get_module("fragment_receiver").connections["output"] = Connection("trb.data_fragment_all")

        # Add the new geoid-to-connections map to the
        # TriggerRecordBuilder.
        old_trb_conf = df_mgraph.get_module("trb").conf
        new_trb_map = old_trb_conf.map + trb_geoid_to_connection
        df_mgraph.reset_module_conf("trb", trb.ConfParams(general_queue_timeout=old_trb_conf.general_queue_timeout,
                                                          reply_connection_name = fragment_connection_name,
                                                          map=trb.mapgeoidconnections(new_trb_map)))

    the_system.app_connections[fragment_endpoint_name] = AppConnection(nwmgr_connection = request_connection_name,
                                                                       receivers = [f"{item[0]}.fragments" for item in df_apps ],
                                                                       topics = [],
                                                                       use_nwqa = False)


def connect_all_fragment_producers(the_system, dataflow_name="dataflow", verbose=False):
    """
    Connect all fragment producers in the system to the appropriate
    queues in the dataflow app.
    """
    for name, app in the_system.apps.items():
        if name==dataflow_name:
            continue
        connect_fragment_producers(name, the_system, verbose)

def resolve_endpoint(app, external_name, inout, verbose=False):
    """
    Resolve an `external` endpoint name to the corresponding internal "module.sinksource"
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

def make_unique_name(base, module_list):
    module_names = [ mod.name for mod in module_list ]
    suffix=0
    while f"{base}_{suffix}" in module_names:
        suffix+=1
    assert f"{base}_{suffix}" not in module_names

    return f"{base}_{suffix}"

def add_network(app_name, the_system, verbose=False):
    """Add the necessary QueueToNetwork and NetworkToQueue objects to the
       application named `app_name`, based on the inter-application
       connections specified in `the_system`. NB `the_system` is modified
       in-place."""

    # if the_system.network_endpoints is None:
    #     the_system.network_endpoints=assign_network_endpoints(the_system)

    if verbose:
        console.log(f"---- add_network2 for {app_name} ----")
    app = the_system.apps[app_name]

    modules_with_network = deepcopy(app.modulegraph.modules)

    unconnected_endpoints = set(app.modulegraph.endpoints.keys())

    if verbose:
        console.log(f"Endpoints to connect are: {unconnected_endpoints}")

    for conn_name, app_connection in the_system.app_connections.items():
        if verbose:console.log(f"conn_name {conn_name}, app_connection {app_connection}")

        # Create the nwmgr connection if it doesn't already exist
        if not the_system.has_network_endpoint(app_connection.nwmgr_connection):
            # IPM connections have the following confusing behaviour:
            # whether the connection is pub/sub or direct connection
            # is determined by whether the list of topics is empty;
            # and the end that binds is upstream for pub/sub
            # connections and downstream for direct connections
            is_pubsub = app_connection.topics != []
            bind_host = app_name if is_pubsub else app_connection.receivers[0].split(".")[0]
            port = the_system.next_unassigned_port()
            address = f"tcp://{{host_{bind_host}}}:{port}"
            if verbose:
                console.log(f"Assigning address {address} for connection {app_connection.nwmgr_connection}")
            the_system.network_endpoints.append(nwmgr.Connection(name=app_connection.nwmgr_connection,
                                                                 topics=app_connection.topics,
                                                                 address=address))
        if not app_connection.use_nwqa:
            if verbose:
                console.log(f"Connection {conn_name} is NetworkManager type; skipping")
            continue
        from_app, from_endpoint = conn_name.split(".", maxsplit=1)

        if from_app == app_name:
            unconnected_endpoints.remove(from_endpoint)
            from_endpoint_internal = resolve_endpoint(app, from_endpoint, Direction.OUT)
            if from_endpoint_internal is None:
                # The module.endpoint for this external endpoint was
                # specified as None, so we assume it was a direct
                # nwmgr sender, and don't make a qton for it
                if verbose:
                    console.log(f"{conn_name} specifies its internal endpoint as None, so not creating a QtoN for it")
                continue
            from_endpoint_module_name, from_endpoint_sink = from_endpoint_internal.split(".")
            # We're a publisher or sender. Make the queue to network
            qton_name = conn_name.replace(".", "_")
            qton_name = make_unique_name(qton_name, modules_with_network)

            if verbose:
                console.log(f"Adding QueueToNetwork named {qton_name} connected to {from_endpoint_internal} in app {app_name}")
            nwmgr_connection_name = app_connection.nwmgr_connection
            nwmgr_connection = the_system.get_network_endpoint(nwmgr_connection_name)
            topic = nwmgr_connection.topics[0] if nwmgr_connection.topics else ""
            modules_with_network.append(DAQModule(name=qton_name,
                                                  plugin="QueueToNetwork",
                                                  connections={}, # No outgoing connections
                                                  conf=qton.Conf(msg_type=app_connection.msg_type,
                                                                 msg_module_name=app_connection.msg_module_name,
                                                                 sender_config=nos.Conf(name=nwmgr_connection_name,
                                                                                        topic=topic))))
            # Connect the module to the QueueToNetwork
            from_endpoint_module = None
            for mod in modules_with_network:
                if mod.name == from_endpoint_module_name:
                    from_endpoint_module = mod
                    break
            mod_connections = from_endpoint_module.connections
            mod_connections[from_endpoint_sink] = Connection(f"{qton_name}.input")

        if verbose:
            console.log(f"app_connection.receivers is {app_connection.receivers}")
        for receiver in app_connection.receivers: 
            to_app, to_endpoint = receiver.split(".", maxsplit=1)
            if to_app == app_name:
                if to_endpoint in unconnected_endpoints:
                    unconnected_endpoints.remove(to_endpoint)
                to_endpoint = resolve_endpoint(app, to_endpoint, Direction.IN)

                ntoq_name = receiver.replace(".", "_")
                ntoq_name = make_unique_name(ntoq_name, modules_with_network)

                if verbose:
                    console.log(f"Adding NetworkToQueue named {ntoq_name} connected to {to_endpoint} in app {app_name}")

                nwmgr_connection_name = app_connection.nwmgr_connection
                nwmgr_connection = the_system.get_network_endpoint(nwmgr_connection_name)
   
                modules_with_network.append(DAQModule(name=ntoq_name,
                                                      plugin="NetworkToQueue",
                                                      connections={"output": Connection(to_endpoint)},
                                                      conf=ntoq.Conf(msg_type=app_connection.msg_type,
                                                                     msg_module_name=app_connection.msg_module_name,
                                                                     receiver_config=nor.Conf(name=nwmgr_connection_name,
                                                                                              subscriptions=nwmgr_connection.topics))))

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
                "DETCHANNELMAPS_SHARE": "getenv",
                "DUNEDAQ_SHARE_PATH": "getenv",
                "TIMING_SHARE": "getenv",
                "LD_LIBRARY_PATH": "getenv",
                "PATH": "getenv",
                # "READOUT_SHARE": "getenv",
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
        # the_system.app_start_order = list(nx.algorithms.dag.topological_sort(app_deps))
        the_system.app_start_order = list(app_deps.nodes)

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
