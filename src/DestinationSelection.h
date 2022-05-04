// Import statements

#ifndef INC_6_08_PATH_FINDING_DESTINATIONSELECTION_H
#define INC_6_08_PATH_FINDING_DESTINATIONSELECTION_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include <mpu6050_esp32.h>

const int NUM_BUILDINGS = 10;
const int NUM_FLOORS = 2;

enum destination_selection_state {IDLE, BUILDING_SELECTION, FLOOR_SELECTION, CONFIRM_DESTINATION, DESTINATION_SELECTED};

class DestinationSelection {

    static const char BUILDINGS[NUM_BUILDINGS][5];
    static const char FLOORS[NUM_FLOORS][5];

    static const int SCROLL_THRESHOLD;
    static const float ANGLE_THRESHOLD;

    destination_selection_state state;
    int destination_building_index;
    int destination_floor_index;

    uint32_t scroll_timer;
    TFT_eSPI* tft;
    MPU6050 imu;

    bool selecting;

    void get_angle(float* angle);
    void clear_selection();
    void display_selection();
public:
    DestinationSelection(TFT_eSPI* _tft);
    void initialize_imu();
    void begin_selection();
    void end_selection();
    void get_destination_building(char* building);
    void get_destination_floor(char* floor);
    int update(int button_flag);
};


#endif //INC_6_08_PATH_FINDING_DESTINATIONSELECTION_H
