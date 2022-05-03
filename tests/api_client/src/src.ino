#include "ApiClient.h"
#include <Arduino.h>
#include <ArduinoJson.h>

const uint16_t LOOP_THRESHOLD = 20000;
uint32_t timer;
ApiClient apiClient;

char USER_ID[] = "team8";
float latitude = -71.0926422;
float longitude = 42.3582736;
int current_floor = 1;
char destination[] = "3";
int destination_floor = 1;


void fetch_current_location() {
    StaticJsonDocument<500> doc = apiClient.fetch_location();

    float latitude = doc["location"]["lat"];
    float longitude = doc["location"]["lng"];

    Serial.printf("Current Location: \n");
    Serial.printf("Latitude: %f \n", latitude);
    Serial.printf("Longitude: %f \n", longitude);
}

void fetch_navigation_instructions() {
    StaticJsonDocument<500> doc = apiClient.fetch_navigation_instructions(USER_ID, latitude, longitude, current_floor,
                                                                          destination, destination_floor);

    const char* curr_building = doc["curr_building"];
    const char* next_building = doc["next_building"];
    const char* curr_node = doc["curr_node"];
    const char* next_node = doc["next_node"];
    float dist_next_node = doc["dist_next_node"];
    float dir_next_node = doc["dir_next_node"];
    bool has_arrived = doc["has_arrived"];
    float eta = doc["eta"];
    const char* dest_node = doc["dest_node"];
    const char* dest_building = doc["dest_building"];

    Serial.printf("Current Building: %s \n", curr_building);
    Serial.printf("Next Building: %s \n", next_building);
    Serial.printf("Current Node: %s \n", curr_node);
    Serial.printf("Next Node: %s \n", next_node);
    Serial.printf("Distance Next Node: %f \n", dist_next_node);
    Serial.printf("Direction Next Node: %f \n", dir_next_node);
    Serial.printf("Has Arrived: %s \n", has_arrived ? "true" : "false");
    Serial.printf("ETA: %f \n", eta);
    Serial.printf("Destination Node: %s \n", dest_node);
    Serial.printf("Destination Building: %s \n", dest_building);
}



void setup() {
    Serial.begin(115200); //start serial at 115200 baud
    while(!Serial);//wait for serial to start

    Serial.println("Calling ApiClient.initialize_wifi_connection()");
    apiClient.initialize_wifi_connection();
    timer = millis();
}

void loop() {
    if (millis() - timer > LOOP_THRESHOLD) {
        Serial.println("-------------------");
        Serial.println("Fetching information...");
        fetch_current_location();
        fetch_navigation_instructions();
        timer = millis();
    }
}
