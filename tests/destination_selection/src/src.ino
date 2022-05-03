#include "DestinationSelection.h"
#include "Button.h"
#include <Arduino.h>


TFT_eSPI tft = TFT_eSPI();
DestinationSelection destination_selection(&tft);

const int BUTTON_PIN = 45;
Button button(BUTTON_PIN);


void setup() {
    Serial.begin(115200); // Set up serial
    while(!Serial);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    tft.init();                             // init screen
    tft.setRotation(2);                     // adjust rotation
    tft.setTextSize(1);                     // default font size, change if you want
    tft.fillScreen(TFT_BLACK);              // fill background
    tft.setTextColor(TFT_GREEN, TFT_BLACK); // set color of font to hot pink foreground, black background

    destination_selection.initialize_imu();
    destination_selection.begin_selection();
}

void loop() {
    uint8_t button_flag = button.update(); // 1 for short press, 2 for long press
    int destination_flag = destination_selection.update(button_flag);

    if (destination_flag == 1) {
        char building[2];
        char floor[2];

        destination_selection.get_destination_building(building);
        destination_selection.get_destination_floor(floor);

        Serial.printf("Building Selected: %s \n", building);
        Serial.printf("Floor Selected: %s \n", floor);
        destination_selection.end_selection();
    }
}
