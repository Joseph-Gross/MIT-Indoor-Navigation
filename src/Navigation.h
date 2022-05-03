// Import statements

#ifndef INC_6_08_PATH_FINDING_NAVIGATION_H
#define INC_6_08_PATH_FINDING_NAVIGATION_H

#include "ApiClient.h"
#include <ArduinoJson.h>
#include "Compass.h"
#include "DestinationSelection.h"
#include <TFT_eSPI.h>

const uint8_t MAX_BUILDING_NAME_LENGTH = 10;
const uint8_t MAX_NODE_ID_LENGTH = 7;
enum class NavigationState {IDLE, LOCATING, ROUTING, NAVIGATING};

struct Location {
    float latitude;
    float longitude;
};

struct NavigationInstructions {
    char curr_building[MAX_BUILDING_NAME_LENGTH];
    char next_building[MAX_BUILDING_NAME_LENGTH];
    char curr_node[MAX_NODE_ID_LENGTH];
    char next_node[MAX_NODE_ID_LENGTH];
    float dist_next_node;
    float dir_next_node;
    bool has_arrived;
    float eta;
    char dest_node[MAX_NODE_ID_LENGTH];
    char dest_building[MAX_NODE_ID_LENGTH];
};


class Navigation {
    static const uint16_t NAVIGATION_UPDATE_LOOP_DURATION;
    static char USER_ID[];

    NavigationState state;
    ApiClient* apiClient;
    Compass* compass;
    TFT_eSPI* tft;

    uint32_t navigation_update_timer;
    bool navigating;

    struct Location location;
    uint8_t current_floor;
    char destination[MAX_BUILDING_NAME_LENGTH];
    uint8_t destination_floor;

    struct NavigationInstructions navigation_instructions;
    void display_navigation_instructions();
    void display_routing_message();
public:
    Navigation(ApiClient* client, Compass* _compass, TFT_eSPI* _tft);
    void fetch_current_location();
    void fetch_navigation_instructions();
    void display_selection();
    void begin_navigation(uint8_t _current_floor, char* _destination, uint8_t _destination_floor);
    void end_navigation();
    int navigate();
};


#endif //INC_6_08_PATH_FINDING_NAVIGATION_H
