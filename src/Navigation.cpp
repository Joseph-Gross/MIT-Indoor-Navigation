#include "Navigation.h"
#include <ArduinoJson.h>

Navigation::Navigation(ApiClient client, Compass _compass){
    apiClient = client;
    compass = _compass;
}

void Navigation::fetchCurrentLocation() {
    DynamicJsonDocument doc(1024);
    apiClient.fetchCurrentLocation(doc);

    float latitude = doc["lat"];
    float longitude = doc["lon"];
    location = {.latitude=latitude, .longitude=longitude};
}

void Navigation::fetchNavigationInstructions() {
    DynamicJsonDocument doc(1024);
    apiClient.fetchNavigationInstructions(doc, user_id, location.latitude, location.longitude, current_floor,
                                          destination, destination_floor);

    char curr_building = doc["curr_building"];
    char next_building = doc["next_building"];
    char curr_node = doc["curr_node"];
    char next_node = doc["next_node"];
    float dist_next_node = doc["dist_next_node"];
    float dir_next_node = doc["dir_next_node"];
    bool has_arrived = doc["has_arrived"];
    float eta = doc["eta"];
    char dest_node = doc["dest_node"];
    char dest_building = doc["dest_building"];

    navigation_instructions = {
            .curr_building=curr_building,
            .next_building=next_building,
            .curr_node=curr_node,
            .next_node=next_node,
            .dist_next_node=dist_next_node,
            .dir_next_node=dir_next_node,
            .has_arrived=has_arrived,
            .eta=eta,
            .dest_node=dest_node,
            .dest_building=dest_building
    };
}

void Navigation::begin_navigation(uint8_t _current_floor, char* _destination, uint8_t _destination_floor) {
    current_floor = _current_floor;
    destination = _destination;
    destination_floor = _destination_floor;
    navigating = true;
}

void Navigation::end_navigation() {
    navigating = false;
}

int Navigation::navigate() {
    flag = 0;

    switch(state) {
        case IDLE:
            if(navigating) {
                state = LOCATION;
                display_routing_message();
            }
            break;

        case LOCATING:
            fetchCurrentLocation();
            state = ROUTING;
            break;

        case ROUTING:
            fetchNavigationInstructions();
            state = NAVIGATING;
            navigation_update_timer = millis();

            if (navigation_instructions.has_arrived) flag = 1;
            display_navigation_instructions();
            compass.update(navigation_instructions.dir_next_node, navigation_instructions.dist_next_node);
            break;

        case NAVIGATING;
            if (!navigating) {
                state = IDLE;
            }
            else if (millis() - navigation_update_timer > NAVIGATION_UPDATE_LOOP_DURATION) {
                state = LOCATING;
            }
            break;
    }
    return flag;
}

void Navigation::display_navigation_instructions() {

}

void Navigation::display_routing_message() {

}