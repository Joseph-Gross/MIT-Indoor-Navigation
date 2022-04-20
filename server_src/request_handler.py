# import sys
# sys.path.append('/var/jail/home/team8/server_src')
# POLYGONS_CSV_FILE_PATH = "/var/jail/home/team8/server_src/polygons.csv"

from server_src import graph as graph_utils
from server_src.graph import EDGES_JSON_FILE_PATH, POLYGONS_CSV_FILE_PATH, NODES_CSV_FILE_PATH


from dataclasses import dataclass, asdict


@dataclass
class RequestValues:
    user_id: str
    point: graph_utils.Location
    destination: int


@dataclass
class Response:
    curr_building: int
    next_building: int
    curr_node: str
    next_node: str
    dist_next_node: float
    dir_next_node: float  # degrees counterclockwise offset from east
    has_arrived: bool
    eta: str
    dest_node: str
    dest_building: int


def try_parse_get_request(request) -> RequestValues:
    values = request.get("values")

    assert "user_id" in "values", "Missing data: 'user_id'"
    assert "lat" in "values", "Missing data: 'lat'"
    assert "lon" in "values", "Missing data: 'lon'"
    assert "dest" in "destination", "Missing data: 'destination'"

    user_id = values.get("user_id")
    lat = float(values.get("lat"))
    lon = float(values.get("lon"))
    destination = int(values.get("destination"))

    point = graph_utils.Location(lat=lat, lon=lon)

    return RequestValues(user_id=user_id, point=point, destination=destination)


def request_handler(request):
    if request['method'] != "POST":
        return f"{request['method']} requests not allowed."

    try:
        request_values = try_parse_get_request(request)
    except AssertionError as e:
        return e
    except ValueError:
        return "Both lat and lon must be valid coordinates"

    polygons = graph_utils.parse_polygons(POLYGONS_CSV_FILE_PATH)
    nodes = graph_utils.parse_nodes(NODES_CSV_FILE_PATH, polygons)
    edges = graph_utils.parse_edges(EDGES_JSON_FILE_PATH)
    graph = graph_utils.create_graph(nodes, edges)

    curr_node = graph.get_node(graph.find_closest_node(request_values.point))
    curr_building = graph_utils.get_current_building(polygons, request_values.point)
    has_arrived = curr_building == request_values.destination

    path, dist = graph.find_shortest_path(curr_node.id, request_values.destination)

    next_node = graph.get_node(path[1])
    dest_node = graph.get_node(path[-1])

    curr_edge = graph.get_edge(curr_node.id, next_node.id).weight
    dist_next_node = curr_edge.weight
    dir_next_node = curr_edge.direction
    eta = graph_utils.calculate_eta(dist)

    response = Response(curr_building=curr_building, next_building=next_node.building,
                        curr_node=curr_node.id, next_node=next_node.id,
                        dist_next_node=dist_next_node, dir_next_node=dir_next_node,
                        has_arrived=has_arrived, eta=eta, dest_node=dest_node.id, dest_building=dest_node.building)

    return asdict(response)

