#include "DestinationSelection.h"
#include "Button.h"
#include <Arduino.h>


const int BUTTON = 45;

TFT_eSPI tft = TFT_eSPI();
DestinationSelection destination_selection(&tft);

Button button(BUTTON);


void setup() {
    Serial.begin(115200); // Set up serial
    while(!Serial);

    Serial.println("Setting up TFT");

    tft.init();                             // init screen
    tft.setRotation(2);                     // adjust rotation
    tft.setTextSize(1);                     // default font size, change if you want
    tft.fillScreen(TFT_BLACK);              // fill background
    tft.setTextColor(TFT_GREEN, TFT_BLACK); // set color of font to hot pink foreground, black background

    Serial.println("Initializing Destination Selection IMU");
    destination_selection.initialize_imu();

    Serial.println("Beginning Selection");
    destination_selection.begin_selection();
}

void loop() {
    int button_flag = button.update();
    int destination_flag = destination_selection.update(button_flag);

    if (button_flag == 1) {
        Serial.printf("%d \n", button_flag);
    }

    if (destination_flag == 1) {
        char* building = destination_selection.get_destination_building();
        char* floor = destination_selection.get_destination_floor();

        Serial.printf("Building Selected: %s \n", building);
        Serial.printf("Floor Selected: %s \n", floor);
        destination_selection.end_selection();
    }
}
