from dataclasses import dataclass
from enum import Enum
from typing import Dict, Set, List, Optional, Tuple

import csv
import json
import pprint
import math

from geopy import distance
from queue import PriorityQueue


# # use these imports if working locally
# POLYGONS_CSV_FILE_PATH = "data/polygons.csv"

# NODES_0_CSV_FILE_PATH = "data/nodes_0.csv"
# NODES_1_CSV_FILE_PATH = "data/nodes_1.csv"
# NODES_STAIRS_CSV_FILE_PATH = "data/nodes_stairs.csv"
# NODES_ELEVATORS_CSV_FILE_PATH = "data/nodes_elevators.csv"

# EDGES_0_CSV_FILE_PATH = "data/edges_0.csv"
# EDGES_1_CSV_FILE_PATH = "data/edges_1.csv"

# these imports are used server side
POLYGONS_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/polygons.csv"

NODES_0_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/nodes_0.csv"
NODES_1_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/nodes_1.csv"
NODES_STAIRS_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/nodes_stairs.csv"
NODES_ELEVATORS_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/nodes_elevators.csv"

EDGES_0_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/edges_0.csv"
EDGES_1_CSV_FILE_PATH = "/var/jail/home/team8/server_src/data/edges_1.csv"


class NodeType(Enum):
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


class Node:

    def __init__(self, id: str, location: Location, floor: int, building_name: Optional[str],
                 node_type: NodeType = NodeType.BUILDING):
        self.id = id
        self.location = location
        self.building = building_name
        self.floor = floor
        self.node_type = node_type

    def __repr__(self):
        return f"Node({self.id}, {self.location}, {self.floor}, {self.building})"


class Edge:

    def __init__(self, v1: Node, v2: Node):
        self.v1 = v1
        self.v2 = v2
        self.weight = self._calculate_distance()
        self.direction = self._calculate_direction()

    def _calculate_distance(self) -> distance.distance.meters:
        return distance.distance(self.v1.location.values, self.v2.location.values).meters

    def _calculate_direction(self) -> Optional[float]:

        point_1 = self.v1.location
        point_2 = self.v2.location

        delta_y = point_2.lat - point_1.lat
        delta_x = point_2.lon - point_1.lon

        return math.atan(delta_y) / delta_x if delta_x != 0 else None


class Graph:

    def __init__(self):
        self._vertices: Dict[str, Node] = dict()   # node_id -> node
        self.buildings: Dict[str, Set[str]] = dict()  # building -> node_id
        self.floors: Dict[int, Set[str]] = dict()  # floor -> node_id
        self.types: Dict[NodeType, Set[str]] = dict()  # NodeType -> node_id
        # adj matrix with weights {node_id: {node_id: weight}}
        self.adj: Dict[str, Dict[str, float]] = dict()

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

    def get_node_ids(self):
        return [key for key in self._vertices]

    def get_weight(self, v1_id: str, v2_id: str):
        return self.adj[v1_id][v2_id]

    def get_closest_node(self, point: Location, floor: int, node_type: NodeType = NodeType.BUILDING) -> str:
        """
        Given location, find closest node in the graph. This will be used to as the start node when calculating the
        shortest path from a location
        """
        floor = 0  # hardcoded to floor 0 for now

        src = Node("s", point, floor, None)

        min_dist = float('inf')
        closest_node = None
        for v_id in self.get_nodes_by_floor(floor):
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

    def find_shortest_path(self, src: str, building_name: str) -> Tuple[List[str], float]:
        assert self.contains_node(src)
        assert self.contains_building(building_name)

        dist, parent = self.sssp(src)

        nodes_in_building = self.get_nodes_by_building(building_name)
        destination = None
        min_dist = float('inf')
        for v in nodes_in_building:
            if dist[v] < min_dist:
                min_dist = dist[v]
                destination = v

        return self.parse_sssp_parent(parent, destination), min_dist


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
                lat, lon = vertex_str.split()
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
            lat, lon = location_str.split()
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
            node = Node(id=node_id, location=location,
                        building_name=building, floor=floor, node_type=node_type)
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
                lat_1, lon_1 = [float(val) for val in raw_points[i].split()]
                lat_2, lon_2 = [float(val) for val in raw_points[i+1].split()]

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

            node_to_add = Node(node_id, node.location, floor,
                               node.building, node.node_type)
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


def get_current_building(polygons: Dict[str, Polygon], point: Location) -> Optional[int]:
    building_num = None
    for building, polygon in polygons.items():  # loop thru all the polygons we got from polygons.csv
        if polygon.is_within_area(point):
            building_num = building
            break

    return building_num


def calculate_eta(distance: float, avg_velocity: float = 1.34112):
    """
    Distance in meters, avg_velocity in m/s
    """

    return distance / avg_velocity


def create_all_graph_components():
    polygons = parse_polygons(POLYGONS_CSV_FILE_PATH)
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
    graph = create_graph(nodes, edges, num_floors=2)
    return polygons, nodes, edges, graph


if __name__ == "__main__":
    polygons = parse_polygons(POLYGONS_CSV_FILE_PATH)
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
    graph = create_graph(nodes, edges, num_floors=2)

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

    test = Location(lon=42.358478, lat=-71.092197)
    print(graph.get_closest_node(test, floor=1))
    dict_of_polys = parse_polygons(POLYGONS_CSV_FILE_PATH)
    building = get_current_building(dict_of_polys, test)
    print(building)
