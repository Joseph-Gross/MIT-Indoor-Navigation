#include "ApiClient.h"
#include "Navigation.h"
#include <TFT_eSPI.h>
#include <Arduino.h>

TFT_eSPI tft = TFT_eSPI();
ApiClient apiClient;
Navigation navigator(&apiClient, &tft);

int current_floor = 1;
char destination[] = "3";
int destination_floor = 1;

void setup() {
    Serial.begin(115200); // Set up serial
    while(!Serial);

    tft.init();                             // init screen
    tft.setRotation(2);                     // adjust rotation
    tft.setTextSize(1);                     // default font size, change if you want
    tft.fillScreen(TFT_BLACK);              // fill background
    tft.setTextColor(TFT_GREEN, TFT_BLACK); // set color of font to hot pink foreground, black background

    apiClient.initialize_wifi_connection();
    navigator.begin_navigation(current_floor, destination, destination_floor);
}

void loop() {
    int navigation_flag = navigator.navigate();

    if (navigation_flag == 1) {
        Serial.println("Ending Navigation");
        navigator.end_navigation();
    }
}
