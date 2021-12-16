from .conf_utils import console, Direction
import networkx as nx

class System:
    """
    A full DAQ system consisting of multiple applications and the
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
        self.app_connections = app_connections if app_connections else dict()
        self.network_endpoints = network_endpoints
        self.app_start_order = app_start_order
        self.digraph = None

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

    def make_digraph(self):
        deps = nx.DiGraph()

        for app_name in self.apps.keys():
            deps.add_node(app_name)

        for from_app_n, from_app in self.apps.items():
            for from_ep in from_app.modulegraph.endpoints.values():
                if from_ep.direction == Direction.OUT:
                    print("OUT", from_app_n, from_ep.external_name)
                    for to_app_n, to_app in self.apps.items():
                        for to_ep in to_app.modulegraph.endpoints.values():
                            if to_ep.direction == Direction.IN:
                                if from_ep.external_name == to_ep.external_name:
                                    deps.add_edge(from_app_n, to_app_n, label=to_ep.external_name)
                elif from_ep.direction == Direction.IN:
                    print("IN", from_app_n, from_ep.external_name)


        return deps


    def export(self, filename):
        self.digraph = self.make_digraph()
        nx.drawing.nx_pydot.write_dot(self.digraph, filename)
