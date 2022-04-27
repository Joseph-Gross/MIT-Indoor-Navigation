#include <ArduinoJson.h>

#ifndef INC_6_08_PATH_FINDING_APICLIENT_H
#define INC_6_08_PATH_FINDING_APICLIENT_H

class ApiClient {

    const uint16_t RESPONSE_TIMEOUT = 6000;
    const char GOOGLE_SERVER[] = "googleapis.com";
    const char TEAM_SERVER[] = "608dev-2.net";
    const char NETWORK[] = "MIT GUEST";
    const char PASSWORD[] = "";

    const char GEOLOCATION_REQUEST_PREFIX[] = "{\"wifiAccessPoints\": [";                 // beginning of json body
    const char GEOLOCATION_REQUEST_SUFFIX[] = "]}";                                       // suffix to POST request
    const char GEOLOCATION_API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8";

    const char *CA_CERT =
            "-----BEGIN CERTIFICATE-----\n"
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

    uint8_t scanning = 1;//set to 1 if you'd like to scan for wifi networks (see below):
    uint8_t channel = 1;                                 // network channel on 2.4 GHz
    byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; // 6 byte MAC address of AP you're targeting.

    uint8_t char_append(char *buff, char c, uint16_t buff_size);
    void parse_response(DynamicJsonDocument* doc, char* response);
public:
    APIClient();
    void initialize_wifi_connection();
    void do_http_request(char *host, char *request, char *response, uint16_t response_size, uint16_t response_timeout, uint8_t serial);
    void do_https_request(char *host, char *request, char *response, uint16_t response_size, uint16_t response_timeout, uint8_t serial);
    int wifi_object_builder(char *object_string, uint32_t os_len, uint8_t channel, int signal_strength, uint8_t *mac_address);
    void fetch_location(DynamicJsonDocument doc);
    void fetch_navigation_instructions(DynamicJsonDocument doc, char* user_id, float lat, float lon,
                                                      uint8_t current_floor, char* destination,
                                                      uint8_t destination_floor);
};


#endif //INC_6_08_PATH_FINDING_APICLIENT_H
