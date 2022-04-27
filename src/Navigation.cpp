// Import statements
#include "Navigation.h"

Navigation::Navigation(ApiClient* client, Compass* _compass, TFT_eSPI* _tft){
    apiClient = client;
    compass = _compass;
    tft = _tft;
}

const uint16_t Navigation::NAVIGATION_UPDATE_LOOP_DURATION = 6000;
const char Navigation::USER_ID[] = "Team8";

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
    int flag = 0;

    switch(state) {
        case NavigationState::IDLE:
            if(NavigationState::navigating) {
                state = NavigationState::LOCATING;
                display_routing_message();
            }
            break;

        case NavigationState::LOCATING:
            fetchCurrentLocation();
            state = NavigationState::ROUTING;
            break;

        case NavigationState::ROUTING:
            fetchNavigationInstructions();
            state = NavigationState::NAVIGATING;
            navigation_update_timer = millis();

            if (navigation_instructions.has_arrived) flag = 1;
            display_navigation_instructions();
            compass.update(navigation_instructions.dir_next_node, navigation_instructions.dist_next_node);
            break;

        case NavigationState::NAVIGATING;
            if (!navigating) {
                state = NavigationState::IDLE;
            }
            else if (millis() - navigation_update_timer > NAVIGATION_UPDATE_LOOP_DURATION) {
                state = NavigationState::LOCATING;
            }
            break;
    }
    return flag;
}

void Navigation::display_navigation_instructions() {
    char destination_str[100];
    char eta_str[100];

    sprintf(output, "Destination: %s", navigation_instructions.destination_name);
    sprintf(eta_str, "ETA: %f minutes", navigation_instructions.eta);

    tft.fillScreen(TFT_BLACK);
    tft.println(destination_str);
    tft.println(eta_str);
}

void Navigation::display_routing_message() {
    tft.fillScreen(TFT_BLACK);
    tft.println("Routing...\n");
    tft.println("Finding the best path to your destination");
}