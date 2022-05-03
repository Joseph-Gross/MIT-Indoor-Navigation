import sys
sys.path.append('/var/jail/home/team8/server_src')
# POLYGONS_CSV_FILE_PATH = "/var/jail/home/team8/server_src/polygons.csv"

# from server_src import graph as graph_utils
import json
import graph as graph_utils
from dataclasses import dataclass, asdict


@dataclass
class RequestValues:
    user_id: str
    point: graph_utils.Location
    destination: str
    current_floor: int
    destination_floor: int


@dataclass
class Response:
    curr_building: str
    next_building: str
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

    assert "user_id" in values, "Missing data: 'user_id'"
    assert "lat" in values, "Missing data: 'lat'"
    assert "lon" in values, "Missing data: 'lon'"
    assert "destination" in values, "Missing data: destination"
    assert "current_floor" in values, "Missing data: current_floor"
    assert "destination_floor" in values, "Missing data: destination_floor"

    user_id = values.get("user_id")
    lat = float(values.get("lat"))
    lon = float(values.get("lon"))
    destination = values.get("destination")
    current_floor = int(values.get("current_floor"))
    destination_floor = int(values.get("destination_floor"))

    point = graph_utils.Location(lat=lat, lon=lon)

    return RequestValues(user_id=user_id, point=point, destination=destination, current_floor=current_floor,
                         destination_floor=destination_floor)


def request_handler(request):
    if request['method'] != "GET":
        return f"{request['method']} requests not allowed."

    try:
        request_values = try_parse_get_request(request)
    except AssertionError as e:
        return e
    except ValueError:
        return "Both lat and lon must be valid coordinates"

    polygons, graph = graph_utils.create_all_graph_components(use_cache=True)
    curr_node = graph.get_node(graph.get_closest_node(request_values.point, floor=request_values.current_floor))
    curr_building = graph_utils.get_current_building(polygons, request_values.point)
    has_arrived = (curr_building == request_values.destination and
                   request_values.current_floor == request_values.destination_floor)

    route = graph.find_shortest_path(curr_node.id, request_values.destination, request_values.destination_floor)

    next_node = graph.get_node(route.path[1])
    dest_node = graph.get_node(route.destination)

    curr_edge = graph.get_edge(curr_node.id, next_node.id)
    dist_next_node = curr_edge.weight
    dir_next_node = curr_edge.direction
    eta = graph_utils.calculate_eta(route.distance)

    response = Response(curr_building=curr_node.building, next_building=next_node.building,
                        curr_node=curr_node.id, next_node=next_node.id,
                        dist_next_node=dist_next_node, dir_next_node=dir_next_node,
                        has_arrived=has_arrived, eta=eta, dest_node=dest_node.id, dest_building=dest_node.building)

    response_dict = asdict(response)
    response_dict["has_arrived"] = int(response.has_arrived)

    return json.dumps(response_dict)

# TODO: Refactor so that building input can be one or two digits (check if numeric, convert to number, then convert to two digit string)
