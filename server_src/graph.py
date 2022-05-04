from dataclasses import dataclass, field, asdict
import time
from enum import Enum, unique
from typing import Dict, Set, List, Optional, Tuple

import csv
import json
import pprint
import math

from geopy.distance import distance, geodesic
from queue import PriorityQueue


# # use these imports if working locally
# POLYGONS_CSV_FILE_PATH = "data/polygons.csv"
#
# NODES_0_CSV_FILE_PATH = "data/nodes_0.csv"
# NODES_1_CSV_FILE_PATH = "data/nodes_1.csv"
# NODES_STAIRS_CSV_FILE_PATH = "data/nodes_stairs.csv"
# NODES_ELEVATORS_CSV_FILE_PATH = "data/nodes_elevators.csv"
#
# EDGES_0_CSV_FILE_PATH = "data/edges_0.csv"
# EDGES_1_CSV_FILE_PATH = "data/edges_1.csv"
#
# GRAPH_JSON_FILE_PATH = "data/graph.json"
# APSP_JSON_FILE_PATH = "data/apsp.json"

# these imports are used server side
POLYGONS_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/polygons.csv"

NODES_0_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/nodes_0.csv"
NODES_1_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/nodes_1.csv"
NODES_STAIRS_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/nodes_stairs.csv"
NODES_ELEVATORS_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/nodes_elevators.csv"

EDGES_0_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/edges_0.csv"
EDGES_1_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/edges_1.csv"

GRAPH_JSON_FILE_PATH = "/var/jail/home/team8/server_src/data/graph.json"
APSP_JSON_FILE_PATH = "/var/jail/home/team8/server_src/data/apsp.json"


@unique
class NodeType(str, Enum):
    BUILDING = "b"
    ELEVATOR = "e"
    STAIR = "s"


@dataclass
class Location:
    lat: float
    lon: float

    @property
    def values(self):
        return self.lat, self.lon


@dataclass
class Route:
    source: str
    destination: str
    path: List[str]
    distance: float


@dataclass
class Node:
    id: str
    location: Location
    building: str
    floor: int
    node_type: Optional[NodeType] = NodeType.BUILDING


@dataclass
class Edge:
    v1: Node
    v2: Node
    weight: float = field(init=False)
    direction: Optional[float] = field(init=False)

    def __post_init__(self):
        self.weight = calculate_distance(self.v1.location, self.v2.location)
        self.direction = calculate_direction(self.v1.location, self.v2.location)


class Graph:

    def __init__(self):
        self._vertices: Dict[str, Node] = dict()   # node_id -> node
        self.buildings: Dict[str, Set[str]] = dict()  # building -> node_id
        self.floors: Dict[int, Set[str]] = dict()  # floor -> node_id
        self.types: Dict[NodeType, Set[str]] = dict()  # NodeType -> node_id
        self.adj: Dict[str, Dict[str, float]] = dict()  # adj matrix with weights {node_id: {node_id: weight}}
        self.apsp_cache: Dict[Tuple[str, str, float], Route] = dict()

    def load_from_json(self, json_file_path: str):
        """
        Loads graph representation to json file.

        {
            node_id: {
                'node': Node,
                'adj': {n1_id: dist, n2_id: dist, ..., ni_id: dist}
            },
        }
        """
        self.__init__()

        with open(json_file_path) as json_file:
            graph = json.load(json_file)

        adj = {}
        for node_id, values in graph.items():
            raw_node_values = values.get('node')
            raw_node_values["location"] = Location(**raw_node_values.get("location"))
            node = Node(**raw_node_values)
            self.add_node(node)

            adj[node_id] = values.get('adj')

        self.adj = adj

    def save_to_json(self, json_file_path: str):
        """
        Dumps graph representation to json file.

        {
            node_id: {
                'node': Node,
                'adj': {n1_id: dist, n2_id: dist, ..., ni_id: dist}
            },
        }
        """
        graph = {}  # node_id -> {node: Node, adj: Adj}

        for node_id, adj in self.adj.items():
            node = self.get_node(node_id)
            graph[node_id] = {
                "node": asdict(node),
                "adj": adj
            }

        with open(json_file_path, 'w') as json_file:
            json.dump(graph, json_file, indent=4, sort_keys=True)

    def load_apsp_from_json(self, json_file_path: str):
        self.apsp_cache.clear()

        with open(json_file_path) as json_file:
            serialized_apsp = json.load(json_file)

        apsp = {tuple(json.loads(key)): Route(**raw_route) for key, raw_route in serialized_apsp.items()}
        self.apsp_cache = apsp

    def save_apsp_to_json(self, json_file_path: str):
        apsp = self.apsp()
        serialized_apsp = {json.dumps(key): asdict(route) for key, route in apsp.items()}

        with open(json_file_path, 'w') as json_file:
            json.dump(serialized_apsp, json_file, indent=4, sort_keys=True)

    def contains_floor(self, floor: int) -> bool:
        return floor in self.floors

    def contains_building(self, building_name: str) -> bool:
        return building_name in self.buildings

    def contains_type(self, node_type: NodeType) -> bool:
        return node_type in self.types

    def contains_node(self, node_id: str) -> bool:
        """
        Checks if graph contains node
        """
        return node_id in self._vertices

    def contains_edge(self, v1_id: str, v2_id: str) -> bool:
        """
        Checks if graph contains edge
        """
        return v1_id in self.adj and v2_id in self.adj[v1_id]

    def add_node(self, node: Node):
        """
        Adds node to graph representation. If node already exists, it will override current value.
        """
        self._vertices[node.id] = node
        self.adj[node.id] = dict()

        if not self.contains_building(node.building):
            self.buildings[node.building] = set()

        if not self.contains_floor(node.floor):
            self.floors[node.floor] = set()

        if not self.contains_type(node.node_type):
            self.types[node.node_type] = set()

        self.buildings[node.building].add(node.id)
        self.floors[node.floor].add(node.id)
        self.types[node.node_type].add(node.id)

    def add_edge(self, v1_id: str, v2_id: str, weight: Optional[float] = None):
        """
        Adds undirected edge (v1, v2) to graph representation
        """
        edge = self.get_edge(v1_id, v2_id)

        self.adj[v1_id][v2_id] = edge.weight if weight is None else weight
        self.adj[v2_id][v1_id] = edge.weight if weight is None else weight

    def get_node(self, node_id: str) -> Node:
        """
        Returns the node object for a node_id
        """
        return self._vertices[node_id]

    def get_edge(self, v1_id: str, v2_id: str) -> Edge:
        v1 = self._vertices[v1_id]
        v2 = self._vertices[v2_id]
        edge = Edge(v1, v2)
        return edge

    def get_neighbors(self, node_id: str) -> List[str]:
        """
        Given src node returns list of neighbor node ids
        """
        return list(self.adj[node_id].keys())

    def get_nodes_by_building(self, building_name: str) -> Set[str]:
        """
        Returns a list of node ids in building
        """
        return self.buildings[building_name]

    def get_nodes_by_floor(self, floor: int) -> Set[str]:
        return self.floors[floor]

    def get_nodes_by_type(self, node_type: NodeType) -> Set[str]:
        return self.types[node_type]

    def get_nodes_by_floor_and_type(self, floor: int, node_type: NodeType):
        nodes_by_floor = self.get_nodes_by_floor(floor)
        nodes_by_type = self.get_nodes_by_type(node_type)
        return nodes_by_floor & nodes_by_type

    def get_nodes_by_building_and_floor_and_type(self, building_name: str, floor: int, node_type: NodeType):
        nodes_by_floor = self.get_nodes_by_floor(floor)
        nodes_by_building = self.get_nodes_by_building(building_name)
        nodes_by_type = self.get_nodes_by_type(node_type)
        return nodes_by_floor & nodes_by_building & nodes_by_type

    def get_node_ids(self) -> Set[str]:
        return set([key for key in self._vertices])

    def get_building_names(self) -> Set[str]:
        return set([key for key in self.buildings])

    def get_floor_numbers(self) -> Set[int]:
        return set([key for key in self.floors])

    def get_node_types(self) -> Set[NodeType]:
        return set([key for key in self.types])

    def get_weight(self, v1_id: str, v2_id: str):
        return self.adj[v1_id][v2_id]

    def get_closest_node(self, point: Location, floor: int = 1, node_type: NodeType = NodeType.BUILDING) -> str:
        """
        Given location, find closest building node in the graph on the same floor. This will be used to as the start
        node when calculating the shortest path from a location
        """
        # hardcoded to floor 0 for now

        src = Node(id="s", location=point, building="None", floor=floor)

        min_dist = float('inf')
        closest_node = None
        for v_id in self.get_nodes_by_floor_and_type(floor, node_type):
            v = self.get_node(v_id)
            edge = Edge(src, v)
            if edge.weight < min_dist:
                min_dist = edge.weight
                closest_node = v

        return closest_node.id

    def sssp(self, src: str):
        """
        Solves sssp from src node on our path.

        src is a valid node id and graph is a valid graph representation

        Returns distance dictionary with shortest distance to every node and parent point dictionary
        """
        assert self.contains_node(src)

        nodes = self.get_node_ids()
        dist = {node_id: float('inf') for node_id in nodes}
        dist[src] = 0

        seen = set()
        pq = PriorityQueue()
        pq.put((0, src))
        parent = dict()

        parent[src] = None
        while not pq.empty():
            (d, current_vertex) = pq.get()
            seen.add(current_vertex)
            for n in self.get_neighbors(current_vertex):
                edge_weight = self.get_weight(current_vertex, n)
                if n not in seen:
                    old_cost = dist[n]
                    new_cost = dist[current_vertex] + edge_weight
                    if new_cost < old_cost:
                        pq.put((new_cost, n))
                        dist[n] = new_cost
                        parent[n] = current_vertex

        return dist, parent

    @staticmethod
    def parse_sssp_parent(parent: Dict[str, str], dest: str):
        """
        Takes in parent pointer dictionary (result of single source shortest path algorithm) and returns a list of
        node ids representing the shortest path to destination.
        """
        path = []
        current_node = dest
        while parent[current_node] is not None:
            path.append(current_node)
            current_node = parent[current_node]
        path.append(current_node)
        return path[::-1]

    def find_shortest_path(self, src: str, building_name: str, floor: int = 1, use_cache: bool = True) -> Route:
        assert self.contains_node(src)
        assert self.contains_building(building_name)

        key = (src, building_name, floor)
        if use_cache and key in self.apsp_cache:
            return self.apsp_cache[key]

        dist, parent = self.sssp(src)

        building_nodes_in_building_and_on_floor = self.get_nodes_by_building_and_floor_and_type(building_name, floor,
                                                                                                NodeType.BUILDING)
        destination = None
        min_dist = float('inf')
        for v in building_nodes_in_building_and_on_floor:
            if dist[v] < min_dist:
                min_dist = dist[v]
                destination = v
        try:
            path = self.parse_sssp_parent(parent, destination)
        except KeyError:
            path = None
            min_dist = float('inf')

        return Route(source=src, destination=destination, path=path, distance=min_dist)

    def apsp(self):
        apsp = {}

        for v in self.get_nodes_by_type(NodeType.BUILDING):
            for building in self.get_building_names():
                for floor in self.get_floor_numbers():
                    route = self.find_shortest_path(v, building, floor)
                    apsp[(v, building, floor)] = route

        return apsp

    def clear_cache(self):
        self.apsp_cache = dict()

    def __eq__(self, other):

        same_node_ids = self.get_node_ids() == other.get_node_ids()
        same_nodes = any([self.get_node(v) == other.get_node(v) for v in self.get_node_ids()]) \
            if same_node_ids else False

        same_buildings = self.get_building_names() == other.get_building_names()
        same_nodes_by_building = all([self.get_nodes_by_building(b) == other.get_nodes_by_building(b)
                                      for b in self.get_building_names()]) if same_buildings else False

        same_floors = self.get_floor_numbers() == other.get_floor_numbers()
        same_nodes_by_floor = all([self.get_nodes_by_floor(f) == other.get_nodes_by_floor(f)
                                   for f in self.get_floor_numbers()]) if same_floors else False

        same_edges = self.adj == other.adj

        return same_nodes and same_nodes_by_building and same_nodes_by_floor and same_edges


class Polygon:
    def __init__(self, vertices: Optional[List[Location]] = None):
        self.vertices = [] if vertices is None else vertices

    def add_vertex(self, vertex: Location):
        self.vertices.append(vertex)

    @property
    def num_vertices(self):
        return len(self.vertices)

    def _get_edges_to_check(self, point: Location):
        """
        returns list of edges that we should check (i.e. y component cross our x-axis)
        """

        edges_to_check = []
        for i in range(self.num_vertices - 1):
            v_1 = self.vertices[i]
            v_2 = self.vertices[i + 1]

            if min(v_1.lon, v_2.lon) < point.lon < max(v_1.lon, v_2.lon):
                edges_to_check.append((i, i + 1))

        v_1 = self.vertices[0]
        v_2 = self.vertices[-1]
        if min(v_1.lon, v_2.lon) < point.lon < max(v_1.lon, v_2.lon):
            edges_to_check.append((0, -1))
        return edges_to_check

    @staticmethod
    def _compute_intersection_point_x_coord(v_1: Location, v_2: Location, point: Location):
        x_1 = v_1.lat - point.lat
        x_2 = v_2.lat - point.lat
        y_1 = v_1.lon - point.lon
        y_2 = v_2.lon - point.lon
        return (x_1 * y_2 - x_2 * y_1) / (y_2 - y_1)

    def is_within_area(self, point: Location):
        """
        Returns whether point is within this polygon's area
        """
        edges_to_check = self._get_edges_to_check(point)

        count = 0
        for v_1_index, v_2_index in edges_to_check:
            v_1 = self.vertices[v_1_index]
            v_2 = self.vertices[v_2_index]

            p = self._compute_intersection_point_x_coord(v_1, v_2, point)
            if p > 0:
                count += 1

        return count % 2 == 1

    def __repr__(self):
        return f"{self.vertices}"


def parse_polygons(polygons_csv_file_path: str) -> Dict[str, Polygon]:
    """
    Returns dictionary mapping building name to polygon representation of building

    CSV structure is assumes to be <Polygon str>, <building num>, <description>
    """
    polygons = dict()
    with open(polygons_csv_file_path) as nodes_csv:
        csv_reader = csv.reader(nodes_csv, delimiter=',')
        line_count = 0
        for row in csv_reader:
            if line_count == 0:
                line_count += 1
                continue

            raw_polygon_str, building, _ = row
            vertices_str = raw_polygon_str[10: -2]
            raw_vertices = vertices_str.split(",")

            vertices = []
            for vertex_str in raw_vertices:
                lon, lat = vertex_str.split()
                vertex = Location(lat=float(lat), lon=float(lon))
                vertices.append(vertex)

            polygons[building] = Polygon(vertices)
            line_count += 1

    return polygons


def parse_nodes(nodes_csv_file_path: str, polygons: Dict[str, Polygon], floor: Optional[int],
                node_type: NodeType = NodeType.BUILDING) -> List[Node]:
    """
    CSV structure is assumes to be <location str>, <building num>, <description>
    """

    nodes = []
    with open(nodes_csv_file_path) as nodes_csv:
        csv_reader = csv.reader(nodes_csv, delimiter=',')
        line_count = 0
        for row in csv_reader:
            if line_count == 0:
                line_count += 1
                continue

            raw_location_str, node_name, _ = row
            location_str = raw_location_str[7: -1]
            lon, lat = location_str.split()
            location = Location(lat=float(lat), lon=float(lon))

            building = None
            for key, polygon in polygons.items():
                if polygon.is_within_area(location):
                    building = key
                    break

            node_name = node_name.split(".")
            node_name.insert(1, str(floor))
            node_name.append(node_type.value)
            node_id = ".".join(node_name)
            node = Node(id=node_id, location=location, building=building, floor=floor, node_type=node_type)
            nodes.append(node)
            line_count += 1

    return nodes


def parse_edges(edges_csv_file_path: str, nodes: List[Node]) -> List[Tuple[str, str]]:

    locations_to_node_ids = dict()
    for node in nodes:
        locations_to_node_ids[node.location.values] = node.id

    edges = []
    with open(edges_csv_file_path) as edges_csv:
        csv_reader = csv.reader(edges_csv, delimiter=',')
        line_count = 0
        for row in csv_reader:
            if line_count == 0:
                line_count += 1
                continue

            raw_line_str, _, _ = row
            line_str = raw_line_str[12: -1]
            raw_points = line_str.split(",")

            for i in range(len(raw_points)-1):
                lon_1, lat_1 = [float(val) for val in raw_points[i].split()]
                lon_2, lat_2 = [float(val) for val in raw_points[i+1].split()]
                node_1_id = locations_to_node_ids[(lat_1, lon_1)]
                node_2_id = locations_to_node_ids[(lat_2, lon_2)]

                edge = (node_1_id, node_2_id)
                edges.append(edge)

            line_count += 1

    return edges


def create_graph(nodes: List[Node], edges: List[Tuple[str, str]], num_floors: int) -> Graph:
    """
    Hard coded list of edges where each edge is a pair of node ids
    """
    graph = Graph()

    for node in nodes:
        if node.node_type == NodeType.BUILDING:
            graph.add_node(node)
            continue

    for v1_id, v2_id in edges:
        if not graph.contains_node(v1_id):
            raise ValueError(f"Invalid edge ({v1_id, v2_id}). Node {v1_id} not in graph")
        if not graph.contains_node(v2_id):
            raise ValueError(f"Invalid edge ({v1_id, v2_id}). Node {v2_id} not in graph")

        if graph.contains_edge(v1_id, v2_id):
            raise ValueError(f"Duplicate edge found. Edge ({v1_id, v2_id}) already exists in graph")

        graph.add_edge(v1_id, v2_id)

    # Handle Stair / Elevator Nodes
    # For each stair / elevator node, add an node to each floor, add edge to losest node on eachh floor, and connect
    # vertically with same "node" with default weight=20
    for node in nodes:
        if node.node_type == NodeType.BUILDING:
            continue

        nodes_to_add = []
        edges_to_add = []
        for floor in range(num_floors):
            node_id = node.id.split(".")
            node_id[1] = str(floor)
            node_id = ".".join(node_id)

            node_to_add = Node(id=node_id, location=node.location, floor=floor,
                               building=node.building, node_type=node.node_type)
            nodes_to_add.append(node_to_add)

            closest_node_id = graph.get_closest_node(node.location, floor)
            edge = (node_id, closest_node_id)
            edges_to_add.append(edge)

        for node_to_add in nodes_to_add:
            graph.add_node(node_to_add)

        for edge in edges_to_add:
            graph.add_edge(*edge)

        # "Altitude" edges (up and down stairs/elevators)
        for i in range(len(nodes_to_add)-1):
            v1_id = nodes_to_add[i].id
            v2_id = nodes_to_add[i+1].id

            graph.add_edge(v1_id, v2_id, weight=20)

    return graph


def create_all_graph_components(num_floors: int = 2, use_cache: bool = True):
    polygons = parse_polygons(POLYGONS_CSV_FILE_PATH)

    if use_cache:
        graph = Graph()
        graph.load_from_json(GRAPH_JSON_FILE_PATH)
        graph.load_apsp_from_json(APSP_JSON_FILE_PATH)
        return polygons, graph

    nodes_stairs = parse_nodes(
        NODES_STAIRS_CSV_FILE_PATH, polygons, None, NodeType.STAIR)
    nodes_elevators = parse_nodes(
        NODES_ELEVATORS_CSV_FILE_PATH, polygons, None, NodeType.ELEVATOR)
    nodes_0 = parse_nodes(NODES_0_CSV_FILE_PATH, polygons, 0)
    nodes_1 = parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)

    edges_0 = parse_edges(EDGES_0_CSV_FILE_PATH, nodes_0)
    edges_1 = parse_edges(EDGES_1_CSV_FILE_PATH, nodes_1)

    nodes = nodes_stairs + nodes_elevators + nodes_0 + nodes_1
    edges = edges_0 + edges_1
    graph = create_graph(nodes, edges, num_floors=num_floors)
    return polygons, graph


def compare_cache_vs_no_cache():
    _, graph = create_all_graph_components(use_cache=False)

    graph.save_to_json(GRAPH_JSON_FILE_PATH)
    graph.save_apsp_to_json(APSP_JSON_FILE_PATH)

    graph_2 = Graph()
    graph_2.load_from_json(GRAPH_JSON_FILE_PATH)
    graph_2.load_apsp_from_json(APSP_JSON_FILE_PATH)

    start = time.time()
    for building in graph.get_building_names():
        graph.find_shortest_path("1.1.1.b", building, use_cache=False)
    end = time.time()
    avg_non_cache_time = (end - start) / len(graph.get_building_names())

    start = time.time()
    for building in graph_2.get_building_names():
        graph_2.find_shortest_path("1.1.1.b", building, use_cache=True)
    end = time.time()
    avg_cache_time = (end - start) / len(graph_2.get_building_names())

    print("Cache vs No Cache Comparison:")
    print(f"Avg time (non cache): {avg_non_cache_time}")
    print(f"Avg time (cache): {avg_cache_time}")

    comparison_factor = round(avg_non_cache_time / avg_cache_time)
    print(f"Cache is faster by a factor of {comparison_factor}")
    print("---------")


def get_current_building(polygons: Dict[str, Polygon], point: Location) -> str:
    building_num = None
    for building, polygon in polygons.items():  # loop thru all the polygons we got from polygons.csv
        if polygon.is_within_area(point):
            building_num = building
            break

    return building_num


def calculate_eta(distance: float, avg_velocity: float = 1.34112):
    """
    Distance in meters, avg_velocity in m/s, eta in seconds
    """

    return round(distance / avg_velocity, 2)


def calculate_distance(point_1: Location, point_2: Location) -> distance.meters:
    return geodesic(point_1.values, point_2.values).meters


def calculate_direction(point_1: Location, point_2: Location) -> Optional[float]:
    delta_y = point_2.lat - point_1.lat
    delta_x = point_2.lon - point_1.lon

    return math.atan(delta_y) / delta_x if delta_x != 0 else None


if __name__ == "__main__":
    polygons = parse_polygons(POLYGONS_CSV_FILE_PATH)
    nodes_stairs = parse_nodes(NODES_STAIRS_CSV_FILE_PATH, polygons, None, NodeType.STAIR)
    nodes_elevators = parse_nodes(NODES_ELEVATORS_CSV_FILE_PATH, polygons, None, NodeType.ELEVATOR)
    nodes_0 = parse_nodes(NODES_0_CSV_FILE_PATH, polygons, 0)
    nodes_1 = parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)

    edges_0 = parse_edges(EDGES_0_CSV_FILE_PATH, nodes_0)
    edges_1 = parse_edges(EDGES_1_CSV_FILE_PATH, nodes_1)

    nodes = nodes_stairs + nodes_elevators + nodes_0 + nodes_1
    edges = edges_0 + edges_1
    polygons, graph = create_all_graph_components(num_floors=2, use_cache=False)

    print("Polygons:")
    pprint.pprint(polygons)
    print("---------")

    print("Nodes:")
    pprint.pprint(nodes)
    print("---------")

    print("Edges:")
    pprint.pprint(edges)
    print("---------")

    print("Weighted Adjacency List:")
    pprint.pprint(graph.adj)
    print("---------")

    print("Path Finding:")
    print(graph.find_shortest_path("7.1.1.b", "2"))
    print("---------")

    compare_cache_vs_no_cache()



