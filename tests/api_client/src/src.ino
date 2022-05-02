#include "ApiClient.h"
#include <Arduino.h>
#include <ArduinoJson.h>

const uint16_t LOOP_THRESHOLD = 2000;
uint32_t timer;
ApiClient apiClient;

char USER_ID[] = "team8";
float latitude = 71.3333;
float longitude = 40.5555;
int current_floor = 1;
char destination[] = "3";
int destination_floor = 1;


void fetch_current_location() {
    StaticJsonDocument<500> doc;
    apiClient.fetch_location(doc);

    float latitude = doc["lat"];
    float longitude = doc["lon"];
}

void fetch_navigation_instructions() {
    StaticJsonDocument<500> doc;
    apiClient.fetch_navigation_instructions(doc, USER_ID, latitude, longitude, current_floor,
                                             destination, destination_floor);

    const char *curr_building = doc["curr_building"];
    const char *next_building = doc["next_building"];
    const char *curr_node = doc["curr_node"];
    const char *next_node = doc["next_node"];
    float dist_next_node = doc["dist_next_node"];
    float dir_next_node = doc["dir_next_node"];
    bool has_arrived = doc["has_arrived"];
    float eta = doc["eta"];
    const char *dest_node = doc["dest_node"];
    const char *dest_building = doc["dest_building"];
}



void setup() {
    Serial.begin(115200); //start serial at 115200 baud
    while(!Serial);//wait for serial to start

    apiClient.initialize_wifi_connection();
}

void loop() {
    if (millis() - timer > LOOP_THRESHOLD) {
        Serial.println("Fetching information");
        fetch_current_location();
        fetch_navigation_instructions();
        timer = millis();
    }
}
