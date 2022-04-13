import graph  # import our graph.py module
POLYGONS_CSV_FILE_PATH = "./polygons.csv"


def get_building(polys, coords):
    building_num = "!No Building Found!"
    for key, value in polys.items():  # loop thru all the polygons we got from polygons.csv
        if value.is_within_area(coords):
            building_num = key
            break

    return str(building_num)


def request_handler(request):
    if "lat" not in request['values'] or "lon" not in request['values']:
        return "You must enter an x and y value."

    # right now it is just a GET request for the curr lon, lat
    if request['method'] == "POST":
        return "POST requests not allowed."

    try:
        coords = (float(request["values"]["lon"]),
                  float(request["values"]["lat"]))
    except:
        return "Both x and y must be valid numbers."

    location_coords = graph.Location(coords[1], coords[0])
    dict_of_polys = graph.parse_polygons(POLYGONS_CSV_FILE_PATH)

    return get_building(dict_of_polys, location_coords)
