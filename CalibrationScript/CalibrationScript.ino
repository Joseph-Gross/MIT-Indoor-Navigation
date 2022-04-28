#include "Compass.h"
TFT_eSPI tft = TFT_eSPI();
TFT_eSPI * tft_ = &tft;
Compass compass(tft_, 80);
float increment = 0.0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200); // Set up serial port
  tft_->init();                            // init screen
  tft_->setRotation(2);                    // adjust rotation
  tft_->setTextSize(1);                    // default font size, change if you want
  tft_->fillScreen(TFT_BLACK);             // fill background
  tft_->setTextColor(TFT_GREEN, TFT_BLACK); // set color of font to hot pink foreground, black background
  compass.initialize();
}

void loop() {
  // put your main code here, to run repeatedly:
  tft_->fillScreen(TFT_BLACK);
  delay(2000);
  compass.calibrate();
  // check the serial monitor and paste the output into 
}
