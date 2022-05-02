#include <Arduino.h>

void setup() {
    Serial.begin(115200); //start serial at 115200 baud
    while(!Serial);//wait for serial to start

    apiClient.initialize_wifi_connection();
}

void loop() {

}
