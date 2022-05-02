// Import statements
#include "ApiClient.h"
#include <WiFiClientSecure.h>
#include <WiFiClient.h>

ApiClient::ApiClient() {}

const uint16_t ApiClient::RESPONSE_TIMEOUT = 6000;
char ApiClient::GOOGLE_SERVER[] = "googleapis.com";
char ApiClient::TEAM_SERVER[] = "608dev-2.net";

// const char ApiClient::GEOLOCATION_REQUEST_PREFIX[] = "{\"wifiAccessPoints\": [";                // beginning of json body
const char ApiClient::GEOLOCATION_REQUEST_PREFIX[] = "{\"wifiAccessPoints\": [";
const char ApiClient::GEOLOCATION_REQUEST_SUFFIX[] = "]}";                                      // suffix to POST request
const char ApiClient::GEOLOCATION_API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8";

const int ApiClient::MAX_APS = 5;

const char ApiClient::CA_CERT[] = "-----BEGIN CERTIFICATE-----\n"
        "MIIDdTCCAl2gAwIBAgILBAAAAAABFUtaw5QwDQYJKoZIhvcNAQEFBQAwVzELMAkG\n"
        "A1UEBhMCQkUxGTAXBgNVBAoTEEdsb2JhbFNpZ24gbnYtc2ExEDAOBgNVBAsTB1Jv\n"
        "b3QgQ0ExGzAZBgNVBAMTEkdsb2JhbFNpZ24gUm9vdCBDQTAeFw05ODA5MDExMjAw\n"
        "MDBaFw0yODAxMjgxMjAwMDBaMFcxCzAJBgNVBAYTAkJFMRkwFwYDVQQKExBHbG9i\n"
        "YWxTaWduIG52LXNhMRAwDgYDVQQLEwdSb290IENBMRswGQYDVQQDExJHbG9iYWxT\n"
        "aWduIFJvb3QgQ0EwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDaDuaZ\n"
        "jc6j40+Kfvvxi4Mla+pIH/EqsLmVEQS98GPR4mdmzxzdzxtIK+6NiY6arymAZavp\n"
        "xy0Sy6scTHAHoT0KMM0VjU/43dSMUBUc71DuxC73/OlS8pF94G3VNTCOXkNz8kHp\n"
        "1Wrjsok6Vjk4bwY8iGlbKk3Fp1S4bInMm/k8yuX9ifUSPJJ4ltbcdG6TRGHRjcdG\n"
        "snUOhugZitVtbNV4FpWi6cgKOOvyJBNPc1STE4U6G7weNLWLBYy5d4ux2x8gkasJ\n"
        "U26Qzns3dLlwR5EiUWMWea6xrkEmCMgZK9FGqkjWZCrXgzT/LCrBbBlDSgeF59N8\n"
        "9iFo7+ryUp9/k5DPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNVHRMBAf8E\n"
        "BTADAQH/MB0GA1UdDgQWBBRge2YaRQ2XyolQL30EzTSo//z9SzANBgkqhkiG9w0B\n"
        "AQUFAAOCAQEA1nPnfE920I2/7LqivjTFKDK1fPxsnCwrvQmeU79rXqoRSLblCKOz\n"
        "yj1hTdNGCbM+w6DjY1Ub8rrvrTnhQ7k4o+YviiY776BQVvnGCv04zcQLcFGUl5gE\n"
        "38NflNUVyRRBnMRddWQVDf9VMOyGj/8N7yy5Y0b2qvzfvGn9LhJIZJrglfCm7ymP\n"
        "AbEVtQwdpf5pLGkkeB6zpxxxYu7KyJesF12KwvhHhm4qxFYxldBniYUr+WymXUad\n"
        "DKqC5JlR3XC321Y9YeRq4VzW9v493kHMB65jUr9TU/Qr6cf9tveCX4XSQRjbgbME\n"
        "HMUfpIBvFSDJ3gyICh3WZlXi/EjJKSZp4A==\n"
        "-----END CERTIFICATE-----\n";

char ApiClient::network[] = "MIT GUEST";
char ApiClient::password[] = "";

uint8_t ApiClient::scanning = 1 ;//set to 1 if you'd like to scan for wifi networks (see below):
uint8_t ApiClient::channel = 1;                                 // network channel on 2.4 GHz
// D4:20:B0:E3:64:E4
byte ApiClient::bssid[] = {0xD4, 0x20, 0xB0, 0xE3, 0x64, 0xE4}; // 6 byte MAC address of AP you're targeting.

char ApiClient::request[IN_BUFFER_SIZE];
char ApiClient::response[OUT_BUFFER_SIZE]; // char array buffer to hold HTTP request
char ApiClient::json_body[JSON_BODY_SIZE];

int ApiClient::max_aps;
int ApiClient::offset = sprintf(json_body, "%s", GEOLOCATION_REQUEST_PREFIX);
int ApiClient::len;

void ApiClient::initialize_wifi_connection() {
    if (scanning){
        int n = WiFi.scanNetworks();
        Serial.println("scan done");
        if (n == 0) {
            Serial.println("no networks found");
        } else {
            Serial.print(n);
            Serial.println(" networks found");
            for (int i = 0; i < n; ++i) {
                Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
                uint8_t* cc = WiFi.BSSID(i);
                for (int k = 0; k < 6; k++) {
                    Serial.print(*cc, HEX);
                    if (k != 5) Serial.print(":");
                    cc++;
                }
                Serial.println("");


                max_aps = max(min(MAX_APS, n), 1);
                for (int i = 0; i < max_aps; ++i)
                {                                                                                                                             // for each valid access point
                    uint8_t *mac = WiFi.BSSID(i);                                                                                             // get the MAC Address
                    offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE - offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); // generate the query
                    if (i != max_aps - 1)
                    {
                        offset += sprintf(json_body + offset, ","); // add comma between entries except trailing.
                    }
                }
            }
        }
    }
    delay(100); //wait a bit (100 ms)

    //if using regular connection use line below:
//    WiFi.begin(network, password);
    //if using channel/mac specification for crowded bands use the following:
    WiFi.begin(network, password, channel, bssid);
    uint8_t count = 0; //count used for Wifi check times
    Serial.print("Attempting to connect to ");
    Serial.println(network);
    while (WiFi.status() != WL_CONNECTED && count<6) {
        delay(500);
        Serial.print(".");
        count++;
    }
    delay(2000);
    if (WiFi.isConnected()) { //if we connected then print our IP, Mac, and SSID we're on
        Serial.println("CONNECTED!");
        Serial.printf("%d:%d:%d:%d (%s) (%s)\n",WiFi.localIP()[3],WiFi.localIP()[2],
                      WiFi.localIP()[1],WiFi.localIP()[0],
                      WiFi.macAddress().c_str() ,WiFi.SSID().c_str());
        delay(500);
    } else { //if we failed to connect just Try again.
        Serial.println("Failed to Connect :/  Going to restart");
        Serial.println(WiFi.status());
        ESP.restart(); // restart the ESP (proper way)
    }
}

/*----------------------------------
   char_append Function:
   Arguments:
      char* buff: pointer to character array which we will append a
      char c:
      uint16_t buff_size: size of buffer buff

   Return value:
      boolean: True if character appended, False if not appended (indicating buffer full)
*/
uint8_t ApiClient::char_append(char *buff, char c, uint16_t buff_size)
{
    int len = strlen(buff);
    if (len > buff_size)
        return false;
    buff[len] = c;
    buff[len + 1] = '\0';
    return true;
}

/*----------------------------------
 * do_http_request Function:
 * Arguments:
 *    char* host: null-terminated char-array containing host to connect to
 *    char* request: null-terminated char-arry containing properly formatted HTTP request
 *    char* response: char-array used as output for function to contain response
 *    uint16_t response_size: size of response buffer (in bytes)
 *    uint16_t response_timeout: duration we'll wait (in ms) for a response from server
 *    uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
 * Return value:
 *    void (none)
 */
void ApiClient::do_http_request(char *host, char *request, char *response, uint16_t response_size, uint16_t response_timeout, uint8_t serial)
{
    if (client2.connect(host, 80))
    { // try to connect to host on port 80
        if (serial)
            Serial.print(request); // Can do one-line if statements in C without curly braces
        client2.print(request);
        memset(response, 0, response_size); // Null out (0 is the value of the null terminator '\0') entire buffer
        uint32_t count = millis();
        while (client2.connected())
        { // while we remain connected read out data coming back
            client2.readBytesUntil('\n', response, response_size);
            if (serial)
                Serial.println(response);
            if (strcmp(response, "\r") == 0)
            { // found a blank line!
                break;
            }
            memset(response, 0, response_size);
            if (millis() - count > response_timeout)
                break;
        }
        memset(response, 0, response_size);
        count = millis();
        while (client2.available())
        { // read out remaining text (body of response)
            char_append(response, client2.read(), response_size);
        }
        if (serial)
            Serial.println(response);
        client2.stop();
        if (serial)
            Serial.println("-----------");
    }
    else
    {
        if (serial)
            Serial.println("connection failed :/");
        if (serial)
            Serial.println("wait 0.5 sec...");
        client2.stop();
    }
}

/*----------------------------------
   do_https_request Function. Similar to do_http_request, but supports https requests
   Arguments:
      char* host: null-terminated char-array containing host to connect to
      char* request: null-terminated char-arry containing properly formatted HTTP request
      char* response: char-array used as output for function to contain response
      uint16_t response_size: size of response buffer (in bytes)
      uint16_t response_timeout: duration we'll wait (in ms) for a response from server
      uint8_t serial: used for printing debug information to terminal (true prints, false doesn't)
   Return value:
      void (none)
*/
void ApiClient::do_https_request(char *host, char *request, char *response, uint16_t response_size, uint16_t response_timeout, uint8_t serial)
{
    client.setHandshakeTimeout(30);
    client.setCACert(CA_CERT); // set cert for https
    if (client.connect(host, 443, 4000))
    { // try to connect to host on port 443
        if (serial)
            Serial.print(request); // Can do one-line if statements in C without curly braces
        client.print(request);
        response[0] = '\0';
        // memset(response, 0, response_size); //Null out (0 is the value of the null terminator '\0') entire buffer
        uint32_t count = millis();
        while (client.connected())
        { // while we remain connected read out data coming back
            client.readBytesUntil('\n', response, response_size);
            if (serial)
                Serial.println(response);
            if (strcmp(response, "\r") == 0)
            { // found a blank line!
                break;
            }
            memset(response, 0, response_size);
            if (millis() - count > response_timeout)
                break;
        }
        memset(response, 0, response_size);
        count = millis();
        while (client.available())
        { // read out remaining text (body of response)
            char_append(response, client.read(), response_size);
        }
        if (serial)
            Serial.println(response);
        client.stop();
        if (serial)
            Serial.println("-----------");
    }
    else
    {
        if (serial)
            Serial.println("connection failed :/");
        if (serial)
            Serial.println("wait 0.5 sec...");
        client.stop();
    }
}

/*----------------------------------
   wifi_object_builder: generates a json-compatible entry for use with Google's geolocation API
   Arguments:
    * `char* object_string`: a char pointer to a location that can be used to build a c-string with a fully-contained JSON-compatible entry for one WiFi access point
    *  `uint32_t os_len`: a variable informing the function how much  space is available in the buffer
    * `uint8_t channel`: a value indicating the channel of WiFi operation (1 to 14)
    * `int signal_strength`: the value in dBm of the Access point
    * `uint8_t* mac_address`: a pointer to the six long array of `uint8_t` values that specifies the MAC address for the access point in question.

      Return value:
      the length of the object built. If not entry is written,
*/
int ApiClient::wifi_object_builder(char *object_string, uint32_t os_len, uint8_t channel, int signal_strength, uint8_t *mac_address)
{
    int out = sprintf(object_string, "{\"macAddress\": \"%02x:%02x:%02x:%02x:%02x:%02x\",\"signalStrength\": %i,\"age\": 0,\"channel\": %d}", mac_address[0], mac_address[1], mac_address[2], mac_address[3], mac_address[4], mac_address[5], signal_strength, channel);
    if (out > os_len)
    {
        sprintf(object_string, "");
        return 0;
    }
    else
    {
        return out;
    }
}

void ApiClient::parse_response(StaticJsonDocument<500> doc, char* response) {

    char* json_start = strchr(response, '{');
    char* json_end = strchr(response, '}');
    int json_len = json_end - json_start;

    char json[json_len + 1];
    strncpy(json, json_start, json_len);
    deserializeJson(doc, json);
}

void ApiClient::fetch_location(StaticJsonDocument<500> doc) {
        sprintf(json_body + offset, "%s", GEOLOCATION_REQUEST_SUFFIX);
        Serial.println(json_body);
        int len = strlen(json_body);
        // Make a HTTP request:
        Serial.println("SENDING REQUEST");
        request[0] = '\0'; // set 0th byte to null
        offset = 0;        // reset offset variable for sprintf-ing
        offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", GEOLOCATION_API_KEY);
        offset += sprintf(request + offset, "Host: googleapis.com\r\n");
        offset += sprintf(request + offset, "Content-Type: application/json\r\n");
        offset += sprintf(request + offset, "cache-control: no-cache\r\n");
        offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
        offset += sprintf(request + offset, "%s\r\n", json_body);
        do_https_request(GOOGLE_SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
        Serial.println("-----------");
        Serial.println(response);
        Serial.println("-----------");
    // }

    parse_response(doc, response);
}

void ApiClient::fetch_navigation_instructions(StaticJsonDocument<500> doc, char* user_id, float lat, float lon,
                                              uint8_t current_floor, char* destination, uint8_t destination_floor) {

    sprintf(request, "GET https://608dev-2.net/sandbox/sc/team8/server_src/request_handler.py?"
                     "user_id=%s&"
                     "lat=%f&"
                     "lon=%f&"
                     "current_floor=%d&"
                     "destination=%s&"
                     "destination_floor=%d  "
                     "HTTP/1.1\r\n", user_id, lat, lon, current_floor, destination, destination_floor);
    strcat(request, "Host: 608dev-2.net\r\n");
    strcat(request, "\r\n");

    do_http_request(TEAM_SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);
    Serial.println("nav response: ");
    Serial.print(response);
    parse_response(doc, response);
}

