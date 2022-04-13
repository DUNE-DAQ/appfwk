from .conf_utils import Direction
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

    def __init__(self, partition_name, apps=None, connections=None, app_start_order=None,
                 first_port=12345):
        self.partition_name = partition_name
        self.apps=apps if apps else dict()
        self.connections = connections if connections else dict()
        self.app_start_order = app_start_order
        self._next_port = first_port
        self.digraph = None

    def __rich_repr__(self):
        yield "partition_name", self.partition_name
        yield "apps", self.apps
        yield "connections", self.connections
        yield "app_start_order", self.app_start_order

    def get_fragment_producers(self):
        """Get a list of all the fragment producers in the system"""
        all_producers = []
        all_geoids = set()
        for app in self.apps.values():
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
                    for to_app_n, to_app in self.apps.items():
                        for to_ep in to_app.modulegraph.endpoints.values():
                            if to_ep.direction == Direction.IN:
                                if from_ep.external_name == to_ep.external_name:
                                    deps.add_edge(from_app_n, to_app_n, label=to_ep.external_name)


        return deps


    def export(self, filename):
        self.digraph = self.make_digraph()
        nx.drawing.nx_pydot.write_dot(self.digraph, filename)

    def next_unassigned_port(self):
        self._next_port += 1
        return self._next_port
    