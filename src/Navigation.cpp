// Import statements
#include "Navigation.h"


Navigation::Navigation(ApiClient* client, Compass* _compass, TFT_eSPI* _tft){
    apiClient = client;
    compass = _compass;
    tft = _tft;
}

const uint16_t Navigation::NAVIGATION_UPDATE_LOOP_DURATION = 10000;
char Navigation::USER_ID[] = "Team8";

void Navigation::fetch_current_location() {
    StaticJsonDocument<500> doc = apiClient->fetch_location();

    float latitude = doc["location"]["lat"];
    float longitude = doc["location"]["lng"];

    if (latitude == 0 | longitude == 0) {
        Serial.println("Invalid Lat/Lon");
        return;
    }

    location.latitude=latitude;
    location.longitude=longitude;
}

void Navigation::fetch_navigation_instructions() {
    StaticJsonDocument<500> doc = apiClient->fetch_navigation_instructions(USER_ID, location.latitude,
                                                                           location.longitude, current_floor,
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

    sprintf(navigation_instructions.curr_building, "%s", curr_building);
    sprintf(navigation_instructions.next_building, "%s", next_building);
    sprintf(navigation_instructions.curr_node, "%s", curr_node);
    sprintf(navigation_instructions.next_node, "%s", next_node);

    navigation_instructions.dist_next_node=dist_next_node;
    navigation_instructions.dir_next_node=dir_next_node;
    navigation_instructions.has_arrived=has_arrived;
    navigation_instructions.eta=eta;

    sprintf(navigation_instructions.dest_node, "%s", dest_node);
    sprintf(navigation_instructions.dest_building, "%s", dest_building);
}

void Navigation::begin_navigation(uint8_t _current_floor, char* _destination, uint8_t _destination_floor) {
    Serial.println("Starting Navigation");
    current_floor = _current_floor;
    strcpy(destination, _destination);
    destination_floor = _destination_floor;
    navigating = true;
    navigation_update_timer = millis();
    Serial.println("Navigation Started");
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
                Serial.println("IDLE -> LOCATING");
            }
            break;

        case NavigationState::LOCATING:
            fetch_current_location();
            state = NavigationState::ROUTING;
            Serial.println("LOCATING -> ROUTING");
            break;

        case NavigationState::ROUTING:
            fetch_navigation_instructions();
            state = NavigationState::NAVIGATING;
            navigation_update_timer = millis();
            Serial.println("ROUTING -> NAVIGATING");

            if (navigation_instructions.has_arrived) flag = 1;
            display_navigation_instructions();
            compass_update_display_timer = millis();
            break;

        case NavigationState::NAVIGATING:
            if (millis() - compass_update_display_timer > 500) {
                compass->update_display(navigation_instructions.dir_next_node);
                compass_update_display_timer = millis();
                display_navigation_instructions();
            }

            if (!navigating) {
                state = NavigationState::IDLE;
                Serial.println("NAVIGATING -> IDLE");
            }
            else if (millis() - navigation_update_timer > NAVIGATION_UPDATE_LOOP_DURATION) {
                state = NavigationState::LOCATING;
                Serial.println("NAVIGATING -> LOCATING");
            }
            break;
    }
    return flag;
}

void Navigation::display_navigation_instructions() {

    char curr_node_str[50];
    char next_node_str[50];
    char dist_str[50];
    char dir_str[50];
    char dest_node_str[50];
    char eta_str[50];

    sprintf(curr_node_str, "Current: %s", navigation_instructions.curr_node);
    sprintf(next_node_str, "Next: %s", navigation_instructions.next_node);
    sprintf(dist_str, "Distance: %f", navigation_instructions.dist_next_node);
    sprintf(dir_str, "Direction: %f", navigation_instructions.dir_next_node);
    sprintf(dest_node_str, "Destination: %s", navigation_instructions.dest_node);
    sprintf(eta_str, "ETA: %d seconds", (int) navigation_instructions.eta);

//    tft->fillScreen(TFT_BLACK);
    tft->setCursor(0, 0, 1);
    tft->println(curr_node_str);
    tft->println(next_node_str);
    tft->println(dist_str);
    tft->println(dir_str);
    tft->println(dest_node_str);
    tft->println(eta_str);
}

void Navigation::display_routing_message() {
    tft->fillScreen(TFT_BLACK);
    tft->setCursor(0, 0, 1);
    tft->println("Routing...\n");
    tft->println("Finding the best path to your destination");
}
