// Import statements
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <string.h>
#include <Adafruit_GFX.h>    // Core graphics library NEEDED FOR COMPASS DISPLAY
#include <Adafruit_ST7735.h> // Hardware-specific library NEEDED FOR COMPASS SENSING
#include <math.h>
#include <Wire.h>

// Define global variables
TFT_eSPI tft = TFT_eSPI();
// Button on pin 45
const int BUTTON = 45;
int32_t timer;


enum global_state {START, ROOM_SELECT, CONFIRM_DESTINATION, NAVIGATING, CONFIRM_CANCEL, ARRIVED};

global_state state = START;
void global_update(int button){
  // As a convention, try to call display_functionwhatever() only when transitioning between states if it makes sense (this won't work for some of them)
  switch (state){
    case START:
      /*
      This is the idle state. A long press will move us to room selection mode.
      */
      if (button != 0) state = ROOM_SELECT;
    break;


    case ROOM_SELECT:
      /* 
      Here we choose the building and room we want to travel to. 
      After both a building and room are selected with short presses, 
      we move to the Confirm Destination state.
      */

    break;


    case CONFIRM_DESTINATION:
      /* 
      we stay in this state until either a short press confirms the destination 
      and we move to navigating, or a long press canceling the destination, 
      and taking us back to room selection
      */

    break;


    case NAVIGATING:
      /* 
      in this state, we are displaying where and how far the destination is. 
      We stay here until the user either arrives at the destination moving us 
      to the arrived state, or the user long presses to cancel the current 
      navigation and moves to the Confirm Cancel Navigation state. 
      */

      if (button == 2) state = CONFIRM_CANCEL;
    break;


    case CONFIRM_CANCEL:
      /* 
      we stay in this state until either a short press confirms that we are 
      canceling the navigation and we move to Start state, 
      or a long press cancels the navigation cancel.
      */


    break;


    case ARRIVED:
      /*
      In the arrived state, we will display an arrived 
      message until a short press moves us back to the Start state.  
      */

    break;
  }
}

Button button(BUTTON); //button object!

void setup(){
  Serial.begin(115200); // Set up serial port

  // Setup wifi, tft, wire, etc.
  wifi_setup(); // need to write this function. Should be easy
  compass_setup(); // need to write this function. Should be easy
  tft_setup(); // need to write this function. Should be extremely easy
  // MAKE MORE SETUP FUNCTIONS

  timer = millis()
}

void loop(){
  int b = button.update();
  global_update(b);
}