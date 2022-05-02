#include "Compass.h"
TFT_eSPI tft = TFT_eSPI();
//Compass compass;
float increment = 0.0;
uint32_t timer = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Set up serial 
  while(!Serial);

  Serial.println("omg11");
  Serial.println("omg33");
  tft.init();        
  Serial.println("omg44");// init screen
  tft.setRotation(2);                    // adjust rotation
  tft.setTextSize(1);  
  Serial.println("omg5");// default font size, change if you want
  tft.fillScreen(TFT_BLACK);             // fill background
  tft.setTextColor(TFT_GREEN, TFT_BLACK); // set color of font to hot pink foreground, black background
  Serial.println("omg22");
  tft.println("Setup TFT");
//  compass.initialize();
  timer = millis();

  Compass compass = Compass(&tft, 80);
}

void loop() {
  // put your main code here, to run repeatedly:
//  tft.fillScreen(TFT_BLACK);
  while ((millis()-timer)<2000){
    
  }
  timer = millis();
//  compass.calibrate();
}
