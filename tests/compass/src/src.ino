#include "Compass.h"
#include <Arduino.h>

TFT_eSPI tft = TFT_eSPI();
Compass compass(&tft, 80);
float increment = 0.0;
uint32_t timer = 0;

void setup() {
    // put your setup code here, to run once:
    Serial.begin(115200); // Set up serial
    while(!Serial);

    tft.init();
    tft.setRotation(2);                    // adjust rotation
    tft.setTextSize(1);
    tft.fillScreen(TFT_BLACK);             // fill background
    tft.setTextColor(TFT_GREEN, TFT_BLACK); // set color of font to hot pink foreground, black background

    Serial.println("Initializing and Calibrating Compass");
    compass.initialize();
//    compass.calibrate();
    Serial.println("Initialization of Compass Complete");

    timer = millis();
}

void loop() {
  compass.refresh_data();                              // This must be done each time through the loop
  compass.calc_quaternion();                           // This must be done each time through the loop
    if (millis()-timer > 1000){
      tft.fillScreen(TFT_BLACK); 
      Serial.println("Updating compass");
      compass.update_display(1, 0);
      
      timer = millis();
    }
    
}
