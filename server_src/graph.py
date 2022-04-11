from enum import Enum
from typing import Dict, Set, List

import csv
import pandas as pd

NODES_CSV_FILE_PATH = "./nodes.csv"
POLYGONS_FILE_PATH = "./polygons.csv"


class Building(Enum):
    ONE: 1
    TWO: 2
    THREE: 3
    FOUR: 4
    FIVE: 5
    SEVEN: 7
    EIGHT: 8
    TEN: 10
    ELEVEN: 11


class Node:

    def __init__(self, id, lat, lon):
        self.id = id
        self.lat = lat
        self.lon = lon
        self.building: Building = Building.ONE

    def get_building(self):
        pass


class Edge:

    def __init__(self, v1: Node, v2: Node):
        self.v1 = v1
        self.v2 = v2
        self.weight = self._calculate_distance()

    def _calculate_distance(self, v1: Node, v2: Node) -> float:
        pass


class Graph:

    def __init__(self):
        self._vertices: Dict[int, Node] = dict()   # id -> node
        self.buildings: Dict[Building, Set[int]] = dict()  # building -> id
        self.adj: Dict[int: Dict[int, float]] = dict()  # adj matrix with weights {node_id: {node_id: weight}}

    def contains_node(self, node_id: int) -> bool:
        """
        Checks if graph contains node
        """
        return node_id in self._vertices

    def add_node(self, node: Node):
        """
        Adds node to graph representation. If node already exists, it will override current value.
        """
        self._vertices[node.id] = node
        self.buildings[node.building].add(node.id)

    def get_node(self, node_id: int) -> Node:
        """
        Returns the node object for a node_id
        """
        return self._vertices[node_id]

    def contains_edge(self, v1: Node, v2: Node) -> bool:
        """
        Checks if graph contains edge
        """
        return v1.id in self.adj and v2.id in self.adj[v1.id]

    def add_edge(self, v1: Node, v2: Node):
        """
        Adds undirected edge (v1, v2) to graph representation
        """
        edge = Edge(v1, v2)
        self.adj[v1.id][v2.id] = edge.weight
        self.adj[v2.id][v1.id] = edge.weight

    def get_edge(self, v1: Node, v2: Node):
        pass

    def get_nodes_by_building(self, building: Building) -> Set[int]:
        """
        Returns a list of node ids in building
        """
        return self.buildings[building]


def parse_nodes() -> List[Node]:
    # with open(NODES_CSV_FILE_PATH) as nodes_csv:
    #     csv_reader = csv.reader(nodes_csv, delimiter=',')
    #     line_count = 0
    #     for row in csv_reader:
    #         if line_count == 0:
    #             print(f'Column names are {", ".join(row)}')
    #             line_count += 1
    #         else:
    #             print(f"{row=}")
    #             line_count += 1
    #     print(f'Processed {line_count} lines.')

    df = pd.read_csv(NODES_CSV_FILE_PATH)

    points = df[["WKT", "name"]]
    print(df)
    print(points.head())



def parse_polygons():
    pass


if __name__ == "__main__":
    parse_nodes()
