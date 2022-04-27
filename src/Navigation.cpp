#include "Navigation.h"
#include <ArduinoJson.h>

Navigation::Navigation(ApiClient client){
    apiClient = client;
}

void Navigation::fetchCurrentLocation() {
    DynamicJsonDocument doc(1024);
    apiClient.fetchCurrentLocation(doc);

    // TODO: Convert doc to Location and save to 'location'
}
void Navigation::fetchNavigationInstructions() {
    DynamicJsonDocument doc(1024);
    apiClient.fetchNavigationInstructions(doc, user_id, location.latitude, location.longitude, current_floor,
                                          destination, destination_floor);

    // TODO: Convert doc to NavigationInstructions and save to 'navigationInstructions'
}
void Navigation::begin_navigation(float lat, float lon, uint8_t _current_floor, char* _destination,
                                  uint8_t _destination_floor) {
    location = {lat, lon};
    current_floor = _current_floor;
    destination = _destination;
    destination_floor = _destination_floor;
    navigating = true;
}

void Navigation::end_navigation() {
    navigating = false;
}

void Navigation::navigate() {
    switch(state) {
        case IDLE:
            if(navigating) {
                state = LOCATION;
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
}

bool Navigation::is_navigating() {
    return navigating;
}

NavigationInstructions Navigation::get_navigation_instructions() {
    return navigation_instructions;
}

void Navigation::display() {

}