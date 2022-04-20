import unittest
import csv

from server_src import graph as graph_utils
from server_src.graph import EDGES_CSV_FILE_PATH, POLYGONS_CSV_FILE_PATH, NODES_CSV_FILE_PATH


class PolygonTests(unittest.TestCase):
    def setUp(self):
        self.polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        
    def test_polygon_parser_0(self):
        """
        Tests

        """
        expected_num_polygons = 11
        num_polygons = len(self.polygons)
        self.assertEqual(expected_num_polygons, num_polygons)

        expected_buildings = ["1", "2", "3", "4", "5", "6", "7", "8", "10", "11", "kc"]
        for building in self.polygons.keys():
            self.assertTrue(building in expected_buildings, f"{building=} not expected")

    def test_polygon_parser_1(self):
        """
        Tests building 10's polygon is parsed as expected and converted to polygon object
        """
        expected_vertices_building_10 = [
            graph_utils.Location(lat=-71.0923882, lon=42.3598751),
            graph_utils.Location(lat=-71.0921938, lon=42.359552),
            graph_utils.Location(lat=-71.0922188, lon=42.3595396),
            graph_utils.Location(lat=-71.0921294, lon=42.3593786),
            graph_utils.Location(lat=-71.09166, lon=42.3595362),
            graph_utils.Location(lat=-71.0919564, lon=42.3600188),
            graph_utils.Location(lat=-71.0923882, lon=42.3598751)
        ]

        building_10 = self.polygons["10"]
        
        self.assertEqual(len(expected_vertices_building_10), len(building_10.vertices))
        
        for vertex in building_10.vertices:
            self.assertTrue(vertex in expected_vertices_building_10, f"{vertex} not in building 10 polygon")


class NodeTests(unittest.TestCase):
    def setUp(self):
        self.nodes = graph_utils.parse_nodes(NODES_CSV_FILE_PATH, polygons={})

    def test_node_ids_unique(self):
        unique_node_ids = set([node.id for node in self.nodes])
        self.assertEqual(len(unique_node_ids), len(self.nodes))
        
    def test_nodes_parse_without_polygons(self):
        nodes_csv = open(NODES_CSV_FILE_PATH)
        csv_reader = csv.reader(nodes_csv)
        expected_num_nodes = len(list(csv_reader)) - 1

        self.assertEqual(expected_num_nodes, len(self.nodes))
    
    def test_nodes_parse_with_polygons(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_CSV_FILE_PATH, polygons)
        
        for node in nodes:
            expected_building_num = node.id.split('.')[0]
            self.assertEqual(expected_building_num, node.building)


class EdgeTests(unittest.TestCase):
    def setUp(self):
        nodes = graph_utils.parse_nodes(NODES_CSV_FILE_PATH, polygons={})
        self.edges = graph_utils.parse_edges(EDGES_CSV_FILE_PATH, nodes)

    def test_edges_unique(self):
        unique_edges = set(self.edges)

        self.assertEqual(len(unique_edges), len(self.edges))


class GraphTests(unittest.TestCase):
    def setUp(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_CSV_FILE_PATH, polygons)
        edges = graph_utils.parse_edges(EDGES_CSV_FILE_PATH, nodes)
        self.graph = graph_utils.create_graph(nodes, edges)


class DijkstraTests(unittest.TestCase):
    def setUp(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_CSV_FILE_PATH, polygons)
        edges = graph_utils.parse_edges(EDGES_CSV_FILE_PATH, nodes)
        self.graph = graph_utils.create_graph(nodes, edges)

    def test_shortest_path_0(self):
        src = "8.2"
        dest = "4.1"

        expected_path = ["8.2", "8.1", "4.3", "4.2", "4.1"]

        dist, parent = self.graph.sssp(src)
        shortest_path = self.graph.parse_sssp_parent(parent, dest)
        
        self.assertEqual(expected_path, shortest_path)

    def test_shortest_path_1(self):
        src = "3.1"
        dest = "4.3"

        expected_path = ["3.1", "kc.2", "4.1", "4.2", "4.3"]
        dist, parent = self.graph.sssp(src)
        shortest_path = self.graph.parse_sssp_parent(parent, dest)

        self.assertEqual(expected_path, shortest_path)

    def test_shortest_path_2(self):
        src = "7.2"
        dest = "10.2"

        expected_path = ["7.2", "7.1", "3.3", "3.4", "10.1", "10.2"]
        dist, parent = self.graph.sssp(src)
        shortest_path = self.graph.parse_sssp_parent(parent, dest)

        self.assertEqual(expected_path, shortest_path)

    def test_shortest_path_3(self):
        src = "11.1"
        dest = "2.1"

        expected_path = ["11.1", "3.3", "3.2", "3.1", "kc.2", "2.1"]
        dist, parent = self.graph.sssp(src)
        shortest_path = self.graph.parse_sssp_parent(parent, dest)

        self.assertEqual(expected_path, shortest_path)

    def test_shortest_path_to_building_0(self):
        src = "3.1"
        dest_building = "2"  # Building number

        expected_path = ["3.1", "kc.2", "4.1", "2.4"]
        expected_dest = "2.4"
        shortest_path, dist = self.graph.find_shortest_path(src, dest_building)
        dest = shortest_path[-1]

        self.assertEqual(expected_path, shortest_path)
        self.assertEqual(expected_dest, dest)
    
    def test_shortest_path_to_building_1(self):
        src = "1.1"
        dest_building = "10"  # Building number

        expected_path = ["1.1", "3.1", "3.2", "3.3", "3.4", "10.1"]
        expected_dest = "10.1"
        shortest_path, dist = self.graph.find_shortest_path(src, dest_building)
        dest = shortest_path[-1]

        self.assertEqual(expected_path, shortest_path)
        self.assertEqual(expected_dest, dest)
    
    def test_shortest_path_to_building_2(self):
        src = "4.4"
        dest_building = "2"  # Building number

        expected_path = ["4.4", "4.3", "4.2", "4.1", "2.4"]
        expected_dest = "2.4"
        shortest_path, dist = self.graph.find_shortest_path(src, dest_building)
        dest = shortest_path[-1]

        self.assertEqual(expected_path, shortest_path)
        self.assertEqual(expected_dest, dest)
    
    def test_shortest_path_to_building_3(self):
        src = "10.2"
        dest_building = "1"  # Building number

        expected_path = ["10.2", "10.1", "3.4", "3.3", "3.2", "3.1", "1.4"]
        expected_dest = "1.4"
        shortest_path, dist = self.graph.find_shortest_path(src, dest_building)
        dest = shortest_path[-1]

        self.assertEqual(expected_path, shortest_path)
        self.assertEqual(expected_dest, dest)


if __name__ == "__main__":
    res = unittest.main(verbosity=3, exit=False)
