from dataclasses import dataclass
from typing import Dict, Set, List, Optional, Tuple
from geopy import distance
import json

import csv
import pprint

import sys
from queue import PriorityQueue
NODES_CSV_FILE_PATH = "./nodes.csv"
POLYGONS_CSV_FILE_PATH = "./polygons.csv"
EDGES_JSON_FILE_PATH = "./edges.json"


@dataclass
class Location:
    lat: float
    lon: float

    @property
    def values(self):
        return self.lat, self.lon


class Node:

    def __init__(self, id: str, location: Location, building: int):
        self.id = id
        self.location = location
        self.building = building

    def __repr__(self):
        return f"Node({self.id}, {self.location}, {self.building})"


class Edge:

    def __init__(self, v1: Node, v2: Node):
        self.v1 = v1
        self.v2 = v2
        self.weight = self._calculate_distance()

    def _calculate_distance(self) -> distance.distance.meters:
        return distance.distance(self.v1.location.values, self.v2.location.values).meters


class Graph:

    def __init__(self):
        self._vertices: Dict[str, Node] = dict()   # id -> node
        self.buildings: Dict[int, Set[int]] = dict()  # building -> id
        self.adj: Dict[str, Dict[str, float]] = dict()  # adj matrix with weights {node_id: {node_id: weight}}

    def contains_building(self, building) -> bool:
        return building in self.buildings

    def contains_node(self, node_id: str) -> bool:
        """
        Checks if graph contains node
        """
        return node_id in self._vertices

    def add_node(self, node: Node):
        """
        Adds node to graph representation. If node already exists, it will override current value.
        """
        self._vertices[node.id] = node
        self.adj[node.id] = dict()

        if not self.contains_building(node.building):
            self.buildings[node.building] = set()

        self.buildings[node.building].add(node.id)

    def get_node(self, node_id: str) -> Node:
        """
        Returns the node object for a node_id
        """
        return self._vertices[node_id]

    def get_neighbors(self, node_id: str) -> List[str]:
        """
        Given src node returns list of neighbor node ids
        """
        return list(self.adj[node_id].keys())

    def contains_edge(self, v1_id: str, v2_id: str) -> bool:
        """
        Checks if graph contains edge
        """
        return v1_id in self.adj and v2_id in self.adj[v1_id]

    def add_edge(self, v1_id: str, v2_id: str):
        """
        Adds undirected edge (v1, v2) to graph representation
        """
        edge = self.get_edge(v1_id, v2_id)
        
        self.adj[v1_id][v2_id] = edge.weight
        self.adj[v2_id][v1_id] = edge.weight

    def get_edge(self, v1_id: str, v2_id: str) -> Edge:
        v1 = self._vertices[v1_id]
        v2 = self._vertices[v2_id]
        edge = Edge(v1, v2)
        return edge

    def get_nodes_by_building(self, building: int) -> Set[int]:
        """
        Returns a list of node ids in building
        """
        return self.buildings[building]

    def get_node_ids(self):
        return [key for key in self._vertices]

    def get_weight(self, v1_id: str, v2_id: str):
        return self.adj[v1_id][v2_id]

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

    def parse_sssp_parent(self, parent: Dict[str, str], dest: str):
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

    def find_shortest_path(self, src: str, building: int):
        assert self.contains_node(src)
        assert self.contains_building(building)

        dist, parent = self.sssp(src)

        nodes_in_building = self.get_nodes_by_building(building)
        destination = None
        min_dist = float('inf')
        for v in nodes_in_building:
            if dist[v] < min_dist:
                min_dist = dist[v]
                destination = v

        return self.parse_sssp_parent(parent, destination)

    def find_closest_node(self, point: Location):
        """
        Given location, find closest node in the graph. This will be used to as the start node when calculating the
        shortest path from a location
        """
        pass


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


def parse_polygons(polygons_csv_file_path: str) -> Dict[int, Polygon]:
    """
    Returns dictionary mapping building number to polygon representation of building.py

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

            raw_polygon_str, building_num, _ = row
            vertices_str = raw_polygon_str[10: -2]
            raw_vertices = vertices_str.split(",")

            vertices = []
            for vertex_str in raw_vertices:
                lat, lon = vertex_str.split()
                vertex = Location(lat=float(lat), lon=float(lon))
                vertices.append(vertex)

            polygons[int(building_num)] = Polygon(vertices)
            line_count += 1

    return polygons


def parse_nodes(nodes_csv_file_path: str, polygons: Dict[int, Polygon]) -> List[Node]:
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

            node = Node(id=node_name, location=location, building=building)
            nodes.append(node)
            line_count += 1

    return nodes


def parse_edges(edges_json_file_path: str) -> List[Tuple[int, int]]:
    """
    Takes in an adjacency list representation of the graph as a JSON file and converts to a list of edges
    """

    edges = set()
    with open(edges_json_file_path) as edges_json_file:
        edges_json = json.load(edges_json_file)

        for v1_id, adj_nodes in edges_json.items():
            for v2_id in adj_nodes:
                if (v1_id, v2_id) in edges or (v2_id, v1_id) in edges:
                    continue

                edge = (v1_id, v2_id)
                edges.add(edge)

    return list(edges)


def create_graph(nodes: List[Node], edges: List[Tuple[int, int]]) -> Graph:
    """
    Hard coded list of edges where each edge is a pair of node ids
    """
    graph = Graph()

    for node in nodes:
        graph.add_node(node)

    for v1_id, v2_id in edges:
        if not graph.contains_node(v1_id):
            raise ValueError(f"Invalid edge ({v1_id, v2_id}). Node {v1_id} not in graph")
        if not graph.contains_node(v2_id):
            raise ValueError(f"Invalid edge ({v1_id, v2_id}). Node {v2_id} not in graph")

        if graph.contains_edge(v1_id, v2_id):
            raise ValueError(f"Duplicate edge found. Edge ({v1_id, v2_id}) already exists in graph")

        graph.add_edge(v1_id, v2_id)

    return graph


if __name__ == "__main__":
    polygons = parse_polygons(POLYGONS_CSV_FILE_PATH)
    nodes = parse_nodes(NODES_CSV_FILE_PATH, polygons)
    edges = parse_edges(EDGES_JSON_FILE_PATH)

    graph = create_graph(nodes, edges)

    print("Polygons:")
    pprint.pprint(polygons)
    print("---------")

    print("Nodes:")
    pprint.pprint(nodes)
    print("---------")

    print("Weighted Adjacency List:")
    pprint.pprint(graph.adj)
    print("---------")

    print("Path Finding:")
    print(graph.find_shortest_path("1.1", 10))
    print("---------")
