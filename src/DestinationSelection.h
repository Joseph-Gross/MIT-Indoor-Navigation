// Import statements

#ifndef INC_6_08_PATH_FINDING_DESTINATIONSELECTION_H
#define INC_6_08_PATH_FINDING_DESTINATIONSELECTION_H

#include <Arduino.h>
#include <TFT_eSPI.h>

const uint8_t MAX_BUILDING_NAME_LENGTH = 25;
enum destination_selection_state {};


class DestinationSelection {

    destination_selection_state state;
    char destination[MAX_BUILDING_NAME_LENGTH];
    int destination_floor;
public:
    DestinationSelection();
    int update();
    void display();
    char* get_destination();
    int get_destination_floor();
};


#endif //INC_6_08_PATH_FINDING_DESTINATIONSELECTION_H
