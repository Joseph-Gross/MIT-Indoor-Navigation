//// Import statements
//
//#ifndef INC_6_08_PATH_FINDING_DESTINATIONSELECTION_H
//#define INC_6_08_PATH_FINDING_DESTINATIONSELECTION_H
//
//#include <Arduino.h>
//#include <TFT_eSPI.h>
//
//const uint8_t MAX_BUILDING_NAME_LENGTH = 25;
//enum destination_selection_state {};
//
//class DestinationSelection {
//
//    destination_selection_state state;
//    char destination[MAX_BUILDING_NAME_LENGTH];
//    int destination_floor;
//    char output[1000];
//    char buildings[50] = "010305071113101204060208162638";
//    char floors[4] = "01";
//
//    // TODO: Consider refactoring so that buildings and floors are a list of chars/ints and the scroll updates the index
//    // of the curr value within the list (for example i would ++ when esp is tilted and modulo num buildings gets applied
//    // to prevent index out of bounds error
//
//public:
//    DestinationSelection();
//    void update(int button);
//    void display();
//    char* get_destination();
//    int get_destination_floor();
//};
//
//
//#endif //INC_6_08_PATH_FINDING_DESTINATIONSELECTION_H
