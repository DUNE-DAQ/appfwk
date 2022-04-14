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
moo.otypes.load_types('networkmanager/nwmgr.jsonnet')
moo.otypes.load_types('iomanager/connection.jsonnet')

from appfwk.utils import acmd, mcmd, mspec
import dunedaq.nwqueueadapters.networkobjectsender as nos
import dunedaq.nwqueueadapters.queuetonetwork as qton
import dunedaq.nwqueueadapters.networkobjectreceiver as nor
import dunedaq.nwqueueadapters.networktoqueue as ntoq
import dunedaq.appfwk.app as appfwk  # AddressedCmd,
import dunedaq.rcif.cmd as rccmd  # AddressedCmd,
import dunedaq.networkmanager.nwmgr as nwmgr
import dunedaq.iomanager.connection as conn

from appfwk.daqmodule import DAQModule

console = Console()

########################################################################
#
# Classes
#
########################################################################

# TODO: Understand whether extra_commands is actually needed. Seems like "resume" is already being sent to everyone?

# TODO: Make these all dataclasses

class Direction(Enum):
    IN = 1
    OUT = 2

class Endpoint:
    # def __init__(self, **kwargs):
    #     if kwargs['connection']:
    #         self.__init_with_nwmgr(**kwargs)
    #     else:
    #         self.__init_with_external_name(**kwargs)
    def __init__(self, external_name, internal_name, direction, topic=[], size_hint=1000):
        self.external_name = external_name
        self.internal_name = internal_name
        self.direction = direction
        self.topic = topic
        self.size_hint = size_hint

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

# AppConnection = namedtuple("AppConnection", ['nwmgr_connection', 'receivers', 'topics', 'msg_type', 'msg_module_name', 'use_nwqa'], defaults=[None, None, True])
AppConnection = namedtuple("AppConnection", ['bind_apps', 'connect_apps'], defaults=[[],[]])

########################################################################
#
# Functions
#
########################################################################

def make_module_deps(modules):
    """
    Given a list of `module` objects, produce a dictionary giving
    the dependencies between them. A dependency is any connection between
    modules. Connections whose upstream ends begin with a '!' are not 
    considered dependencies, to allow us to break cycles in the DAG.

    Returns a networkx DiGraph object where nodes are module names
    """

    deps = nx.DiGraph()
    for module in modules:
        deps.add_node(module.name)

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

def make_system_connections(the_system):
    """Given a system with defined apps and endpoints, create the 
    set of connections that satisfy the endpoints.

    If an endpoint's ID only exists for one application, a queue will
    be used, otherwise a network connection will be created.

    If a queue connection has a single producer and single consumer, it will use FollySPSC,
    otherwise FollyMPMC will be used.

    
    """

    endpoint_map = defaultdict(list)

    for app in the_system.apps:
      the_system.connections[app] = []
      for endpoint in the_system.apps[app].modulegraph.endpoints:
        console.log(f"Adding endpoint {endpoint.external_name}, app {the_system.apps[app].name}, direction {endpoint.direction}")
        endpoint_map[endpoint.external_name] += [{"app": the_system.apps[app].name, "endpoint": endpoint}]

    console.log(endpoint_map.items())
    for endpoint_name,endpoints in endpoint_map.items():
        console.log(f"Processing {endpoint_name} with defined endpoints {endpoints}")
        if len(endpoints) < 2:
            raise ValueError(f"Connection with name {endpoint_name} only has one endpoint!")
        first_app = endpoints[0]["app"]
        in_apps = []
        out_apps = []
        size = 0
        topics = {}
        for endpoint in endpoints:
            direction = endpoint['endpoint'].direction
            topics.update(endpoint['endpoint'].topic)
            console.log(f"Direction is {direction}")
            if direction == Direction.IN: 
                in_apps += [endpoint["app"]]
            else: 
                out_apps += [endpoint["app"]]
            if endpoint['endpoint'].size_hint > size:
                size = endpoint['endpoint'].size_hint

        if len(in_apps) == 0:
            raise ValueError(f"Connection with name {endpoint_name} has no producers!")
        if len(out_apps) == 0:
            raise ValueError(f"Connection with name {endpoint_name} has no consumers!")

        if all(first_app == elem["app"] for elem in endpoints):
            if len(in_apps) == 1 and len(out_apps) == 1:
                console.log(f"Connection {endpoint_name}, SPSC Queue")
                the_system.connections[first_app] += [conn.ConnectionId(uid=endpoint_name, service_type="kQueue", data_type="", uri=f"queue://FollySPSC:{size}")]
            else:
                console.log(f"Connection {endpoint_name}, MPMC Queue")
                the_system.connections[first_app] += [conn.ConnectionId(uid=endpoint_name, service_type="kQueue", data_type="", uri=f"queue://FollyMPMC:{size}")]
        else:
            if len(topics) == 0:
                console.log(f"Connection {endpoint_name}, Network")
                if len(in_apps) > 1:
                    raise ValueError(f"Connection with name {endpoint_name} has multiple receivers, which is unsupported for a network connection!")
                the_system.app_connections[endpoint_name] = AppConnection(bind_apps=in_apps, connect_apps=out_apps)
                port = the_system.next_unassigned_port()
                address = f"tcp://{{host_{in_apps[0]}}}:{port}"
                the_system.connections[in_apps[0]] += [conn.ConnectionId(uid=endpoint_name, service_type="kNetwork", data_type="", uri=address)]
                for app in set(out_apps):
                    the_system.connections[app] += [conn.ConnectionId(uid=endpoint_name, service_type="kNetwork", data_type="", uri=address)]
            else:
                console.log(f"Connection {endpoint_name}, Pub/Sub")
                for app in set(out_apps):
                    the_system.app_connections[endpoint_name] = AppConnection(bind_apps=[app], connect_apps=in_apps)
                    port = the_system.next_unassigned_port()
                    address = f"tcp://{{host_{app}}}:{port}"
                    the_system.connections[app] += [conn.ConnectionId(uid=f"{endpoint_name}.{app}", service_type="kPubSub", data_type="", uri=address, topics=topics)]
                    for in_app in set(in_apps):
                        the_system.connections[in_app] += [conn.ConnectionId(uid=f"{endpoint_name}.{app}", service_type="kPubSub", data_type="", uri=address, topics=topics)]
        
         

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

    if len(system.connections) == 0:
        make_system_connections(system)

    app_connrefs = defaultdict(list)
    for endpoint in app.modulegraph.endpoints:
        module, name = endpoint.internal_name.split(".")
        console.log(f"module, name= {module}, {name}, endpoint.external_name={endpoint.external_name}, endpoint.direction={endpoint.direction}")
        app_connrefs[module] += [conn.ConnectionRef(name=name, uid=endpoint.external_name, dir= "kInput" if endpoint.direction == Direction.IN else "kOutput")]

    if verbose:
        console.log(f"Creating mod_specs for {[ (mod.name, mod.plugin) for mod in modules ]}")
    mod_specs = [ mspec(mod.name, mod.plugin, app_connrefs[mod.name]) for mod in modules ]

    # Fill in the "standard" command entries in the command_data structure

    command_data['init'] = appfwk.Init(modules=mod_specs, connections=system.connections[app.name])

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
        console.log(f"---- add_network for {app_name} ----")
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
        from_app, from_endpoint = conn_name.split(".", maxsplit=1)

        if from_app == app_name:
            if from_endpoint in unconnected_endpoints:
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
                to_endpoint_internal = resolve_endpoint(app, to_endpoint, Direction.IN)
                if to_endpoint_internal is None:
                    # The module.endpoint for this external endpoint was
                    # specified as None, so we assume it was a direct
                    # nwmgr sender, and don't make a ntoq for it
                    if verbose:
                        console.log(f"{to_endpoint} specifies its internal endpoint as None, so not creating a NtoQ for it")
                    continue

                ntoq_name = receiver.replace(".", "_")
                ntoq_name = make_unique_name(ntoq_name, modules_with_network)

                if verbose:
                    console.log(f"Adding NetworkToQueue named {ntoq_name} connected to {to_endpoint_internal} in app {app_name}")

                nwmgr_connection_name = app_connection.nwmgr_connection
                nwmgr_connection = the_system.get_network_endpoint(nwmgr_connection_name)

                modules_with_network.append(DAQModule(name=ntoq_name,
                                                      plugin="NetworkToQueue",
                                                      connections={"output": Connection(to_endpoint_internal)},
                                                      conf=ntoq.Conf(msg_type=app_connection.msg_type,
                                                                     msg_module_name=app_connection.msg_module_name,
                                                                     receiver_config=nor.Conf(name=nwmgr_connection_name,
                                                                                              subscriptions=nwmgr_connection.topics))))

    if unconnected_endpoints:
        # TODO: Use proper logging
        console.log(f"Warning: the following endpoints of {app_name} were not connected to anything: {unconnected_endpoints}")

    app.modulegraph.modules = modules_with_network

def generate_boot(apps: list, partition_name="${USER}_test", ers_settings=None, info_svc_uri="file://info_${APP_ID}_${APP_PORT}.json",
                  disable_trace=False, use_kafka=False, verbose=False, extra_env_vars=dict()) -> dict:
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
            "DUNEDAQ_ERS_DEBUG_LEVEL": "getenv_ifset",
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

    boot["exec"]["daq_application"]["env"].update(extra_env_vars)
    boot["exec"]["daq_application_ups"]["env"].update(extra_env_vars)

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
    if verbose:
        console.log(f"make_app_json for app {app_name}")
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
