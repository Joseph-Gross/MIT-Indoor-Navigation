// Import statements

#ifndef INC_6_08_PATH_FINDING_APICLIENT_H
#define INC_6_08_PATH_FINDING_APICLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

const uint16_t IN_BUFFER_SIZE = 5000;  // size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 5000; // size of buffer to hold HTTP response
const uint16_t JSON_BODY_SIZE = 5000;

class ApiClient {

    static const uint16_t RESPONSE_TIMEOUT;
    static char GOOGLE_SERVER[];
    static char TEAM_SERVER[];

    static const char GEOLOCATION_REQUEST_PREFIX[];
    static const char GEOLOCATION_REQUEST_SUFFIX[];
    static const char GEOLOCATION_API_KEY[];

    static const int MAX_APS;
    static const char CA_CERT[];

    static char network[];
    static char password[];
    static uint8_t scanning;
    static uint8_t channel;
    static byte bssid[];

    WiFiClientSecure client; // global WiFiClient Secure object
    WiFiClient client2;      // global WiFiClient Secure object

    static char request[IN_BUFFER_SIZE];
    static char response[OUT_BUFFER_SIZE]; // char array buffer to hold HTTP request
    static char json_body[JSON_BODY_SIZE];

    static int max_aps;
    static int offset;
    static int len;

    uint8_t char_append(char *buff, char c, uint16_t buff_size);
    void parse_response(StaticJsonDocument<1000>* doc, char* response);
public:
    ApiClient();
    void initialize_wifi_connection();
    void do_http_request(char *host, char *request, char *response, uint16_t response_size, uint16_t response_timeout, uint8_t serial);
    void do_https_request(char *host, char *request, char *response, uint16_t response_size, uint16_t response_timeout, uint8_t serial);
    int wifi_object_builder(char *object_string, uint32_t os_len, uint8_t channel, int signal_strength, uint8_t *mac_address);
    void fetch_location(StaticJsonDocument<1000>* doc);
    void fetch_navigation_instructions(StaticJsonDocument<1000>* doc, char* user_id, float lat, float lon,
                                       uint8_t current_floor, char* destination, uint8_t destination_floor);
};


#endif //INC_6_08_PATH_FINDING_APICLIENT_H
