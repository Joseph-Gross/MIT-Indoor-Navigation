// Import statements

#ifndef INC_6_08_PATH_FINDING_BUTTON_H
#define INC_6_08_PATH_FINDING_BUTTON_H

#include <Arduino.h>
#include <stdint.h>

enum button_state {S0,S1,S2,S3,S4};

class Button{
public:
    uint32_t S2_start_time;
    uint32_t button_change_time;
    uint32_t debounce_duration;
    uint32_t long_press_duration;
    uint8_t pin;
    uint8_t flag;
    uint8_t button_pressed;
    button_state state; // This is public for the sake of convenience
    Button(int p);
    void read();
    int update();
};



#endif //INC_6_08_PATH_FINDING_BUTTON_H
