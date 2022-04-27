#include "ApiClient.h"

#ifndef INC_6_08_PATH_FINDING_NAVIGATION_H
#define INC_6_08_PATH_FINDING_NAVIGATION_H

enum navigation_state {IDLE, LOCATING, ROUTING, NAVIGATING};

const uint8_t MAX_NODE_ID_LENGTH = 7;
const uint8_t MAX_BUILDING_NAME_LENGTH = 25;

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
    const uint16_t NAVIGATION_UPDATE_LOOP_DURATION = 6000;
    const char USER_ID[] = "Team8";

    navigation_state state;
    ApiClient apiClient;

    uint32_t navigation_update_timer;
    bool navigating;

    struct Location location;
    uint8_t current_floor;
    char destination[MAX_BUILDING_NAME_LENGTH];
    uint8_t destination_floor;

    struct NavigationInstructions navigation_instructions;
public:
    Navigation(ApiClient client);
    void fetchCurrentLocation();
    void fetchNavigationInstructions();
    void begin_navigation(float lat, float lon, uint8_t _current_floor, char* _destination, uint8_t _destination_floor);
    void end_navigation();
    void navigate();
    bool is_navigating();
    NavigationInstructions get_navigation_instructions():
    void display();
};


#endif //INC_6_08_PATH_FINDING_NAVIGATION_H
