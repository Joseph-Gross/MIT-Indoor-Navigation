// Import statements
#include "Button.h"
#include "Compass.h"
#include "Navigation.h"
#include "DestinationSelection.h"
#include "ApiClient.h"

#include <SPI.h>
#include <TFT_eSPI.h>
#include <string.h>
#include <math.h>


const int BUTTON_PIN = 45;
Button button(BUTTON_PIN);

// Define global variables
TFT_eSPI tft = TFT_eSPI();
ApiClient apiClient;
Compass compass(&tft, 80);
Navigation navigator(&apiClient, &compass, &tft);
DestinationSelection destination_selection(&tft, &compass);

uint8_t current_floor = 1;
char destination_building[2];
char destination_floor[2];

enum global_state
{
  START,
  DESTINATION_SELECTION,
  NAVIGATING,
  CONFIRM_CANCEL,
  ARRIVED
};

global_state state = START;

int destination_selection_flag;
int navigation_flag;


void display_start_message()
{
  tft.fillScreen(BACKGROUND);
  tft.setCursor(0, 0, 1);
  tft.println("Welcome to TinyTim, \nMIT's premiere \ncampus navigator\n");
  tft.println("Press the button to \nactivate your guide \nand select a \ndestination.");
}

void global_update(int button_flag){
  switch (state){
    case START:
      if (button_flag != 0) {
          state = DESTINATION_SELECTION;
          destination_selection.begin_selection();
          Serial.println("START -> DESTINATION_SELECTION");
      }
      break;
    case DESTINATION_SELECTION:
      /* 
      Here we choose the building and room we want to travel to. 
      After both a building and room are selected with short presses, 
      we move to the Confirm Destination state.
      */
      destination_selection_flag = destination_selection.update(button_flag);

      if (destination_selection_flag == 1) {
          destination_selection.get_destination_building(destination_building);
          destination_selection.get_destination_floor(destination_floor);

          Serial.printf("Building Selected: %s \n", destination_building);
          Serial.printf("Floor Selected: %s \n", destination_floor);
          destination_selection.end_selection();
          state = NAVIGATING;
          Serial.println("DESTINATION_SELECTION -> NAVIGATING");

          int _destination_floor = atoi(destination_floor);
          navigator.begin_navigation(current_floor, destination_building, _destination_floor);
      }
      break;
    case NAVIGATING:
      /* 
      in this state, we are displaying where and how far the destination is. 
      We stay here until the user either arrives at the destination moving us 
      to the arrived state, or the user long presses to cancel the current 
      navigation and moves to the Confirm Cancel Navigation state. 
      */
      navigation_flag = navigator.navigate();
      if (navigation_flag == 1) {
          Serial.println("NAVIGATING -> ARRIVED");
          state = ARRIVED;
          navigator.end_navigation();
          tft.fillScreen(BACKGROUND);
          tft.println("You have arrived! Press button to return to start. \n");
      }
      else if (button_flag == 2) {
          Serial.println("NAVIGATING -> CONFIRM_CANCEL");
          state = CONFIRM_CANCEL;
          tft.fillScreen(BACKGROUND);
          tft.println("Exit navigation? Short press if yes, long press if no. \n");
      }
      break;
    case CONFIRM_CANCEL:
      /* 
      we stay in this state until either a short press confirms that we are 
      canceling the navigation and we move to Start state, 
      or a long press cancels the navigation cancel.
      */
      if (button_flag == 1) state = START;
      if (button_flag == 2) state = NAVIGATING;
      break;
    case ARRIVED:
      /*
      In the arrived state, we will display an arrived 
      message until a short press moves us back to the Start state.  
      */
      if (button_flag != 0) {
          state = START;
          Serial.println("ARRIVED -> START");
          display_start_message();
      }
      break;
  }
}

void setup() {
    Serial.begin(115200); // Set up serial
    while(!Serial);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    tft.init();                             // init screen
    tft.setRotation(2);                     // adjust rotation
    tft.setTextSize(1);                     // default font size, change if you want
    tft.fillScreen(TFT_BLACK);              // fill background
    tft.setTextColor(TFT_GREEN, TFT_BLACK); // set color of font to hot pink foreground, black background
    tft.println("Tiny Tim booting... \nplease wait\n\n");

    tft.println("Initializing Wifi...");
    apiClient.initialize_wifi_connection();
    
    tft.println("Initializing Compass...");
    compass.initialize();
    
    display_start_message();
}

void loop(){
  compass.refresh_data();                              // This must be done each time through the loop
  compass.calc_quaternion();                           // This must be done each time through the loop
  int button_flag = button.update();
  global_update(button_flag);
}
