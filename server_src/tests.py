import unittest
import csv

#from server_src import graph as graph_utils
import graph as graph_utils
#from server_src.graph import POLYGONS_CSV_FILE_PATH, NODES_1_CSV_FILE_PATH, EDGES_1_CSV_FILE_PATH, GRAPH_JSON_FILE_PATH, \
 #   APSP_JSON_FILE_PATH
from graph import POLYGONS_CSV_FILE_PATH, NODES_1_CSV_FILE_PATH, EDGES_1_CSV_FILE_PATH, GRAPH_JSON_FILE_PATH, \
    APSP_JSON_FILE_PATH
from geopy.distance import geodesic as GD
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
            self.assertTrue(building in expected_buildings, f"{building} not expected")
            #this said building= changed it to just building hope that is ok
    def test_polygon_parser_1(self):
        """
        Tests building 10's polygon is parsed as expected and converted to polygon object
        """
        expected_vertices_building_10 = [
            graph_utils.Location(lat=42.3598751, lon=-71.0923882),
            graph_utils.Location(lat=42.359552, lon=-71.0921938),
            graph_utils.Location(lat=42.3595396, lon=-71.0922188),
            graph_utils.Location(lat=42.3593786, lon=-71.0921294),
            graph_utils.Location(lat=42.3595362, lon=-71.09166),
            graph_utils.Location(lat=42.3600188, lon=-71.0919564),
            graph_utils.Location(lat=42.3598751, lon=-71.0923882)
        ]

        building_10 = self.polygons["10"]
        
        self.assertEqual(len(expected_vertices_building_10), len(building_10.vertices))
        
        for vertex in building_10.vertices:
            self.assertTrue(vertex in expected_vertices_building_10, f"{vertex} not in building 10 polygon")


class NodeTests(unittest.TestCase):
    def setUp(self):
        self.nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons={}, floor=1)

    def test_node_ids_unique(self):
        unique_node_ids = set([node.id for node in self.nodes])
        self.assertEqual(len(unique_node_ids), len(self.nodes))
        
    # def test_nodes_parse_without_polygons(self):
    #     nodes_csv = open(NODES_1_CSV_FILE_PATH)
    #     csv_reader = csv.reader(nodes_csv)
    #     expected_num_nodes = len(list(csv_reader)) - 1

    #     self.assertEqual(expected_num_nodes, len(self.nodes))
    
    def test_nodes_parse_with_polygons(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)
        
        for node in nodes:
            expected_building_num = node.id.split('.')[0]
            self.assertEqual(expected_building_num, node.building)


class EdgeTests(unittest.TestCase):
    def setUp(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)
        self.edges = graph_utils.parse_edges(EDGES_1_CSV_FILE_PATH, nodes)

    def test_edges_unique(self):
        unique_edges = set(self.edges)

        self.assertEqual(len(unique_edges), len(self.edges))


class GraphTests(unittest.TestCase):
    def setUp(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)
        edges = graph_utils.parse_edges(EDGES_1_CSV_FILE_PATH, nodes)
        self.graph = graph_utils.create_graph(nodes, edges, num_floors=2)

    def test_save_and_load_graph_json(self):
        self.graph.save_to_json(GRAPH_JSON_FILE_PATH)

        graph = graph_utils.Graph()
        graph.load_from_json(GRAPH_JSON_FILE_PATH)

        self.assertEqual(self.graph, graph)

    def test_save_and_load_apsp_json(self):
        self.graph.save_to_json(GRAPH_JSON_FILE_PATH)
        self.graph.save_apsp_to_json(APSP_JSON_FILE_PATH)

        graph = graph_utils.Graph()
        graph.load_from_json(GRAPH_JSON_FILE_PATH)
        graph.load_apsp_from_json(APSP_JSON_FILE_PATH)

        self.assertEqual(self.graph.apsp(), graph.apsp_cache)

class ClosestNodeTests(unittest.TestCase):
    def setUp(self):
        self.polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        self.nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, self.polygons, 1)
        self.edges = graph_utils.parse_edges(EDGES_1_CSV_FILE_PATH, self.nodes)
        self.graph = graph_utils.create_graph(self.nodes, self.edges, num_floors=2)

    def test_closest_node_1(self):
        #tests if current loc is a node if it correctly gets that node
        for node in self.nodes:
            current_loc = node.location
            self.assertEqual(self.graph.get_closest_node(current_loc),node.id)
    def test_closest_node_2(self):
        #test when you are in building 1 but not on a node
        loc_from_google = graph_utils.Location(lat=42.3579114810823, lon=-71.09195835623672)
        #do manual calculation for closest node
        #closest node should be 1.1.1.b from google maps
        #print(distance.distance(loc_from_google.values,nodes["1.1.1.b")]))
        node_1 = None
        node_2 = None
        for n in self.nodes:
            if n.id == "1.1.1.b":
                node_1 = n
            if n.id == "1.1.2.b":
                node_2 = n
        #print(node_1.location.values)
        #print(loc_from_google.values)
        #print("Between current location and 1.1.1.b:"+str(distance.distance(loc_from_google.values,node_1.location.values)))
        self.assertEqual(self.graph.get_closest_node(loc_from_google),"1.1.1.b")

class DijkstraTests(unittest.TestCase):
    def setUp(self):
        polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
        nodes = graph_utils.parse_nodes(NODES_1_CSV_FILE_PATH, polygons, 1)
        edges = graph_utils.parse_edges(EDGES_1_CSV_FILE_PATH, nodes)
        self.graph = graph_utils.create_graph(nodes, edges, num_floors=2)
    def test_shortest_path_0(self):
        src = "8.1.2.b"
        dest = "4.1.1.b"
        expected_path = ["8.1.2.b", "8.1.1.b", "4.1.3.b", "4.1.2.b", "4.1.1.b"]

        dist, parent = self.graph.sssp(src)
        shortest_path = self.graph.parse_sssp_parent(parent, dest)
        
        self.assertEqual(expected_path, shortest_path)

    def test_shortest_path_1(self):
        src = "3.1.1.b"
        dest = "4.1.3.b"

        expected_path = ["3.1.1.b", "kc.1.2.b", "4.1.1.b", "4.1.2.b", "4.1.3.b"]
        dist, parent = self.graph.sssp(src)
        shortest_path = self.graph.parse_sssp_parent(parent, dest)

        self.assertEqual(expected_path, shortest_path)

    def test_shortest_path_2(self):
        src = "7.1.2.b"
        dest = "10.1.2.b"

        expected_path = ["7.1.2.b", "7.1.1.b", "3.1.3.b", "3.1.4.b", "10.1.1.b", "10.1.2.b"]
        dist, parent = self.graph.sssp(src)
        shortest_path = self.graph.parse_sssp_parent(parent, dest)

        self.assertEqual(expected_path, shortest_path)

    def test_shortest_path_3(self):
        src = "11.1.1.b"
        dest = "2.1.1.b"

        expected_path = ["11.1.1.b", "3.1.3.b", "3.1.2.b", "3.1.1.b", "kc.1.2.b", "2.1.1.b"]
        dist, parent = self.graph.sssp(src)
        shortest_path = self.graph.parse_sssp_parent(parent, dest)

        self.assertEqual(expected_path, shortest_path)

    def test_shortest_path_to_building_0(self):
        src = "3.1.1.b"
        dest_building = "2"  # Building number

        expected_path = ["3.1.1.b", "kc.1.2.b", "2.1.1.b"]
        expected_dest = expected_path[-1]
        route = self.graph.find_shortest_path(src, dest_building)

        self.assertEqual(expected_path, route.path)
        self.assertEqual(expected_dest, route.destination)
    
    def test_shortest_path_to_building_1(self):
        src = "1.1.1.b"
        dest_building = "10"  # Building number

        expected_path = ["1.1.1.b", "3.1.1.b", "3.1.2.b", "3.1.3.b", "3.1.4.b", "10.1.1.b"]
        expected_dest = expected_path[-1]
        route = self.graph.find_shortest_path(src, dest_building)

        self.assertEqual(expected_path, route.path)
        self.assertEqual(expected_dest, route.destination)
    
    def test_shortest_path_to_building_2(self):
        src = "4.1.4.b"
        dest_building = "2"  # Building number

        expected_path = ["4.1.4.b", "4.1.3.b", "4.1.2.b", "4.1.1.b", "2.1.4.b"]
        expected_dest = expected_path[-1]
        route = self.graph.find_shortest_path(src, dest_building)

        self.assertEqual(expected_path, route.path)
        self.assertEqual(expected_dest, route.destination)
    
    def test_shortest_path_to_building_3(self):
        src = "10.1.2.b"
        dest_building = "1"  # Building number

        expected_path = ["10.1.2.b", "10.1.1.b", "3.1.4.b", "3.1.3.b", "3.1.2.b", "3.1.1.b", "1.1.4.b"]
        expected_dest = expected_path[-1]
        route = self.graph.find_shortest_path(src, dest_building)

        self.assertEqual(expected_path, route.path)
        self.assertEqual(expected_dest, route.destination)


if __name__ == "__main__":
    #unittest.main()
    res = unittest.main(verbosity=3, exit=False)
