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

uint8_t current_floor = 1;
uint8_t destination_floor = 1;
char destination[MAX_BUILDING_NAME_LENGTH] = "3";


enum global_state {START, ROOM_SELECT, CONFIRM_DESTINATION, NAVIGATING, CONFIRM_CANCEL, ARRIVED};

global_state state = START;
global_state previous_state = ARRIVED;

ApiClient apiClient();
Button button(BUTTON);
Compass compass();
Navigation navigator(apiClient, compass);

DestinationSelection destination_selector();


void display_start_message() {
    tft.fillScreen(BACKGROUND);
    tft.println("Welcome to TinyTim, MIT's premiere campus navigator\n");
    tft.println("Press the button to activate your guide and select a destination.");
}

void display_confirm_destination_message() {
    tft.fillScreen(BACKGROUND);
    tft.println("You selected the following destination \n");
    tft.println(dest);
    tft.println('\nShort press to continue. Long press to reselect.\n')
}

void display_destination_selection_instructions() {
    tft.fillScreen(BACKGROUND);
    tft.println("Tilt screen for number scrolling. \n");
    tft.println("Short press to confirm destination. \n");
}

void global_update(int button){
  switch (state){
    case START:
      if (button != 0) {
          state = ROOM_SELECT;
          display_destination_selection_instructions();
      }
    break;

    case ROOM_SELECT:
      /* 
      Here we choose the building and room we want to travel to. 
      After both a building and room are selected with short presses, 
      we move to the Confirm Destination state.
      */
      destination_selector.update();
      
      if (button != 0) {
        state = CONFIRM_DESTINATION;
        destination_floor = destination_selector.get_destination_floor();
        destination = destination_selector.get_destination();
        display_confirm_destination_message();
      }
    break;

    case CONFIRM_DESTINATION:
      /* 
      we stay in this state until either a short press confirms the destination 
      and we move to navigating, or a long press canceling the destination, 
      and taking us back to room selection
      */
      if (button == 1) {
          state = NAVIGATING;
          navigator.begin_navigation(current_floor, destination, destination_floor);
      }
      if (button == 2) state = ROOM_SELECT;
    break;


    case NAVIGATING:
      /* 
      in this state, we are displaying where and how far the destination is. 
      We stay here until the user either arrives at the destination moving us 
      to the arrived state, or the user long presses to cancel the current 
      navigation and moves to the Confirm Cancel Navigation state. 
      */
      int navigation_flag = navigator.navigate();
      if (navigation_flag == 1) {
          state = ARRIVED;
          navigator.end_navigation();
      }

      // NOTE: navigator.navigate() calls compass.update(dir, dist) which should call compass.display()
      if (button == 2) state = CONFIRM_CANCEL;

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
      if (button != 0) {
          state = START;
          display_start_message();
      }
      break;
  }
}



void setup(){
  Serial.begin(115200); // Set up serial port

  // Setup wifi, tft, wire, etc.
  apiClient.initialize_wifi_connection();
  compass.initialize();
  tft_setup(); // need to write this function. Should be extremely easy
  // MAKE MORE SETUP FUNCTIONS

  display_start_message();

  timer = millis()
}

void loop(){
  int button_flag = button.update();
  global_update(button_flag);
}