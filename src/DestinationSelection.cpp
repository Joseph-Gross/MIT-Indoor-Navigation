#include "DestinationSelection.h"

int mod(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}

const char DestinationSelection::BUILDINGS[NUM_BUILDINGS][5] = {"1", "2", "3", "4", "5", "6", "7", "8", "10", "11"};
const char DestinationSelection::FLOORS[NUM_FLOORS][5] = {"0", "1"};

const int DestinationSelection::SCROLL_THRESHOLD = 150;
const float DestinationSelection::ANGLE_THRESHOLD = 0.3;

DestinationSelection::DestinationSelection(TFT_eSPI* _tft){
    state = IDLE;
    destination_building_index = -1;
    destination_floor_index = -1;
    scroll_timer = millis();
    tft = _tft;
}

void DestinationSelection::get_angle(float* angle) {
    imu.readAccelData(imu.accelCount);
    *angle = imu.accelCount[1] * imu.aRes;
}

void DestinationSelection::clear_selection() {
    destination_building_index = -1;
    destination_floor_index = -1;
}

void DestinationSelection::initialize_imu() {
    if (imu.setupIMU(1)) {
        Serial.println("IMU Connected!");
    } else {
        Serial.println("IMU Not Connected :/");
        Serial.println("Restarting");
        ESP.restart(); // restart the ESP (proper way)
    }
}

void DestinationSelection::begin_selection() {
    selecting = true;
}

void DestinationSelection::end_selection() {
    selecting = false;
}

void DestinationSelection::display_selection(){
    tft->fillScreen(TFT_BLACK);
    tft->setCursor(0, 0, 1);
    tft->println("Destination Selection \n\n");

    tft->printf("Building: %s\n\n", destination_building_index >= 0 ? BUILDINGS[destination_building_index] : "");
    tft->printf("Floor: %s\n\n\n\n", destination_floor_index >= 0 ? FLOORS[destination_floor_index] : "");
}

void DestinationSelection::get_destination_building(char* building) {
    strcpy(building, BUILDINGS[destination_building_index]);
}

void DestinationSelection::get_destination_floor(char* floor) {
    strcpy(floor, FLOORS[destination_floor_index]);
}

int DestinationSelection::update(int button_flag) {
    float angle;
    get_angle(&angle);
    int flag = 0;

    switch(state) {
        case IDLE:
            if (selecting) {
                state = BUILDING_SELECTION;
                display_selection();
            }
            break;

        case BUILDING_SELECTION:
            if (abs(angle) > ANGLE_THRESHOLD) {
                if (millis() - scroll_timer > SCROLL_THRESHOLD) {
                    if (angle > 0) {
                        destination_building_index++;
                    } else {
                        destination_building_index--;
                    }

                    destination_building_index = mod(destination_building_index, NUM_BUILDINGS);
                    scroll_timer = millis();
                    display_selection();
                }
            }

            if (button_flag == 1) {
                state = FLOOR_SELECTION;
            }
            break;

        case FLOOR_SELECTION:
            if (abs(angle) > ANGLE_THRESHOLD) {
                if (millis() - scroll_timer > SCROLL_THRESHOLD) {
                    if (angle > 0) {
                        destination_floor_index++;
                    } else {
                        destination_floor_index--;
                    }

                    destination_floor_index = mod(destination_floor_index, NUM_FLOORS);
                    scroll_timer = millis();
                    display_selection();
                }
            }

            if (button_flag == 1) {
                state = CONFIRM_DESTINATION;
                tft->println("Confirm Selection:\n");
                tft->println("Short Press: confirm");
                tft->println("Long Press: cancel\n\n");
            } else if (button_flag == 2) {
                clear_selection();
                display_selection();
                state = BUILDING_SELECTION;
            }
            break;

        case CONFIRM_DESTINATION:
            if (button_flag == 1) {
                flag = 1;
                state = DESTINATION_SELECTED;
                tft->println("Confirmed!!");
            } else if (button_flag == 2) {
                clear_selection();
                display_selection();
                state = BUILDING_SELECTION;
            }
            break;

        case DESTINATION_SELECTED:
            if (!selecting) {
                clear_selection();
                state = IDLE;
            }
    }

    return flag;
}