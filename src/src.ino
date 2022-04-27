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

#include <Button.h>
#include <Compass.h>
#include "Navigation.h"
#include "DestinationSelection.h"
#include "APIClient.h"

#define BACKGROUND TFT_BLACK

// Define global variables
TFT_eSPI tft = TFT_eSPI();
// Button on pin 45
const int BUTTON = 45;
int32_t timer;
char dest[20];
char prev_dest[20] = "";



enum global_state {START, ROOM_SELECT, CONFIRM_DESTINATION, NAVIGATING, CONFIRM_CANCEL, ARRIVED};

global_state state = START;
global_state previous_state = ARRIVED;

ApiClient apiClient();
Button button(BUTTON); //button object!
Navigation navigator(apiClient); //navigation object
Compass compass(); //compass object


void display_start_message() {
    tft.fillScreen(BACKGROUND);
    tft.println("Welcome to TinyTim, MIT's premiere campus navigator\n");
    tft.println("Press the button to activate your guide and select a destination.");
}

void global_update(int button){
  // As a convention, try to call display_functionwhatever() only when transitioning between states if it makes sense (this won't work for some of them)
  switch (state){
    case START:
      /*
      This is the idle state. Any press will move us to room selection mode.
      */
      if (previous_state!=START){
        display_start_message();
        previous_state = START;
        Serial.println("START -> ")
      }
      if (button != 0) state = ROOM_SELECT;
    break;

    case ROOM_SELECT:
      /* 
      Here we choose the building and room we want to travel to. 
      After both a building and room are selected with short presses, 
      we move to the Confirm Destination state.
      */
      if (previous_state!=ROOM_SELECT){
        tft.fillScreen(BACKGROUND);
        tft.println("Tilt screen for number scrolling. \n");
        tft.println("Short press to confirm destination. \n");
        previous_state = ROOM_SELECT;
      }
      // dest = select_destination();
      // if (strcmp(dest,prev_dest)!=0){
        // tft.setCursor(20,0,1); // whatever cursor location prevents it from 
        // tft.println(dest);
        // prev_dest = dest;
      // }
      
      if (button != 0) {
        state = CONFIRM_DESTINATION;
      }
    break;

    case CONFIRM_DESTINATION:
      /* 
      we stay in this state until either a short press confirms the destination 
      and we move to navigating, or a long press canceling the destination, 
      and taking us back to room selection
      */
      if (previous_state!=CONFIRM_DESTINATION){
        tft.fillScreen(BACKGROUND);
        tft.println("You selected the following destination \n");
        tft.println(dest);
        tft.println('\nShort press to continue. Long press to reselect.\n')
        previous_state = CONFIRM_DESTINATION;
      }
      if (button == 1) state = NAVIGATING;
      if (button == 2) state = ROOM_SELECT;
    break;


    case NAVIGATING:
      /* 
      in this state, we are displaying where and how far the destination is. 
      We stay here until the user either arrives at the destination moving us 
      to the arrived state, or the user long presses to cancel the current 
      navigation and moves to the Confirm Cancel Navigation state. 
      */
      if (previous_state!=NAVIGATING){
        tft.fillScreen(BACKGROUND);
        tft.println("Navigation in progress \n");
        tft.println(dest);
        tft.println('\nLong press button to exit.\n')
        previous_state = NAVIGATING;
      }
      // figure out current location
      // navigator.curr_loc(); // or whatever this function would be
      // make an http request for the path
      // response = navigator.http_request(dest, other_paramters);
      // char parsed[100] = navigator.parse_response() // or whatever function does this and whatever the variable should contain
      // compass.update(dir_next_node, distance); // this should call arrow.update() internally? how will this display?
      if (button == 2) state = CONFIRM_CANCEL;
      // if (navigation.arrived()) state = ARRIVED; // or whatever function does this.
    break;


    case CONFIRM_CANCEL:
      /* 
      we stay in this state until either a short press confirms that we are 
      canceling the navigation and we move to Start state, 
      or a long press cancels the navigation cancel.
      */
      if (previous_state!=CONFIRM_CANCEL){
        tft.fillScreen(BACKGROUND);
        tft.println("Exit navigation? Short press if yes, long press if no. \n");
        previous_state = CONFIRM_CANCEL;
      }
      if (button == 1) state = START;
      if (button == 2) state = NAVIGATING;
    break;


    case ARRIVED:
      /*
      In the arrived state, we will display an arrived 
      message until a short press moves us back to the Start state.  
      */
      if (previous_state!=ARRIVED){
        tft.fillScreen(BACKGROUND);
        tft.println("You have arrived! Press button to return to start. \n");
        previous_state = ARRIVED;
      }
      if (button != 0) state = START;
    break;
  }
}



void setup(){
  Serial.begin(115200); // Set up serial port

  // Setup wifi, tft, wire, etc.
  apiClient.initialize_wifi_connection();
  compass_setup(); // need to write this function. Should be easy
  tft_setup(); // need to write this function. Should be extremely easy
  // MAKE MORE SETUP FUNCTIONS

  timer = millis()
}

void loop(){
  int button_flag = button.update();
  global_update(b);
}