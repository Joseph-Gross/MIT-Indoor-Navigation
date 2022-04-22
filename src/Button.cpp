#include "Button.h"


Button::Button(int p){
    flag = 0;
    state = S0;
    pin = p;
    S2_start_time = millis(); //init
    button_change_time = millis(); //init
    debounce_duration = 10;
    long_press_duration = 1000;
    button_pressed = 0;
}

void Button::read() {
    uint8_t button_val = digitalRead(pin);
    button_pressed = !button_val; //invert button
}

int Button::update() {
    read();
    flag = 0;

    switch (state) {
        case S0:
            if (button_pressed) {
                state = S1;
                button_change_time = millis();
            }
            break;
        case S1:
            if (button_pressed & millis()-button_change_time >= debounce_duration) {
                state = S2;
                S2_start_time = millis();
            } else if (!button_pressed) {
                state = S0;
                button_change_time = millis();
            }
            break;
        case S2:
            if (button_pressed & millis()-S2_start_time >= long_press_duration) {
                state = S3;
            } else if (!button_pressed) {
                state = S4;
                button_change_time = millis();
            }
            break;
        case S3:
            if (!button_pressed) {
                state = S4;
                button_change_time = millis();
            }
            break;
        case S4:
            if (!button_pressed & millis()-button_change_time >= debounce_duration) {
                if (millis()-S2_start_time < long_press_duration) {
                    flag = 1;
                } else {
                    flag = 2;
                }
                state = S0;
            } else if (button_pressed & millis()-S2_start_time < long_press_duration) {
                state = S2;
                button_change_time = millis();
            } else if (button_pressed & millis()-S2_start_time >= long_press_duration) {
                state = S3;
                button_change_time = millis();
            }
            break;
    }

    return flag;
}
