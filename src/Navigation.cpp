// Import statements
#include "Navigation.h"

Navigation::Navigation(ApiClient* client, Compass* _compass, TFT_eSPI* _tft){
    apiClient = client;
    compass = _compass;
    tft = _tft;
}

const uint16_t Navigation::NAVIGATION_UPDATE_LOOP_DURATION = 6000;
char Navigation::USER_ID[] = "Team8";

void Navigation::fetch_current_location() {
    StaticJsonDocument<500> doc;
    apiClient->fetch_location(doc);

    float latitude = doc["lat"];
    float longitude = doc["lon"];

    location.latitude=latitude;
    location.longitude=longitude;
}

void Navigation::fetch_navigation_instructions() {
    StaticJsonDocument<500> doc;
    apiClient->fetch_navigation_instructions(doc, USER_ID, location.latitude, location.longitude, current_floor,
                                             destination, destination_floor);

    const char* curr_building = doc["curr_building"];
    const char* next_building = doc["next_building"];
    const char* curr_node = doc["curr_node"];
    const char* next_node = doc["next_node"];
    float dist_next_node = doc["dist_next_node"];
    float dir_next_node = doc["dir_next_node"];
    bool has_arrived = doc["has_arrived"];
    float eta = doc["eta"];
    const char* dest_node = doc["dest_node"];
    const char* dest_building = doc["dest_building"];

    sprintf(navigation_instructions.curr_building, curr_building);
    sprintf(navigation_instructions.next_building, next_building);
    sprintf(navigation_instructions.curr_node, curr_node);
    sprintf(navigation_instructions.next_building, next_building);

    navigation_instructions.dist_next_node=dist_next_node;
    navigation_instructions.dir_next_node=dir_next_node;
    navigation_instructions.has_arrived=has_arrived;
    navigation_instructions.eta=eta;

    sprintf(navigation_instructions.dest_node, dest_node);
    sprintf(navigation_instructions.dest_building, dest_building);
}

void Navigation::begin_navigation(uint8_t _current_floor, char* _destination, uint8_t _destination_floor) {
    current_floor = _current_floor;
    strcpy(destination, _destination);
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
            if(navigating) {
                state = NavigationState::LOCATING;
                display_routing_message();
            }
            break;

        case NavigationState::LOCATING:
            fetch_current_location();
            state = NavigationState::ROUTING;
            break;

        case NavigationState::ROUTING:
            fetch_navigation_instructions();
            state = NavigationState::NAVIGATING;
            navigation_update_timer = millis();

            if (navigation_instructions.has_arrived) flag = 1;
            display_navigation_instructions();
            compass->update(navigation_instructions.dir_next_node, navigation_instructions.dist_next_node);
            break;

        case NavigationState::NAVIGATING:
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

    sprintf(destination_str, "Destination: %s", navigation_instructions.dest_building);
    sprintf(eta_str, "ETA: %f minutes", navigation_instructions.eta);

    tft->fillScreen(TFT_BLACK);
    tft->println(destination_str);
    tft->println(eta_str);
}

void Navigation::display_routing_message() {
    tft->fillScreen(TFT_BLACK);
    tft->println("Routing...\n");
    tft->println("Finding the best path to your destination");
}
