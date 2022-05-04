import unittest
import csv

#from server_src import graph as graph_utils
import graph as graph_utils
#from server_src.graph import POLYGONS_CSV_FILE_PATH, NODES_1_CSV_FILE_PATH, EDGES_1_CSV_FILE_PATH, GRAPH_JSON_FILE_PATH, \
 #   APSP_JSON_FILE_PATH
from graph import POLYGONS_CSV_FILE_PATH, NODES_1_CSV_FILE_PATH, EDGES_1_CSV_FILE_PATH, GRAPH_JSON_FILE_PATH, \
    APSP_JSON_FILE_PATH
class ClosestNodeTests(unittest.TestCase):
    def setUp(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)
        edges = graph_utils.parse_edges(EDGES_1_CSV_FILE_PATH, nodes)
        self.graph = graph_utils.create_graph(nodes, edges, num_floors=2)

    def test_closest_node_1(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)
        for node in nodes:
            current_loc = node.location
            self.assertEqual(self.graph.get_closest_node(current_loc),node.id)
    #def closest_node_2(self):
        
class DirectionTests(unittest.TestCase):
    def setUp(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)
        edges = graph_utils.parse_edges(EDGES_1_CSV_FILE_PATH, nodes)
        self.graph = graph_utils.create_graph(nodes, edges, num_floors=2)
class DijkstraTests(unittest.TestCase):
    def setUp(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)
        edges = graph_utils.parse_edges(EDGES_1_CSV_FILE_PATH, nodes)
        self.graph = graph_utils.create_graph(nodes, edges, num_floors=2)
    def test_shortest_path_0(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)
        for node in nodes:
            current_loc = node.location
            self.assertEqual(self.graph.get_closest_node(current_loc),node.id)
if __name__ == "__main__":
    #unittest.main()
    res = unittest.main(verbosity=3, exit=False)