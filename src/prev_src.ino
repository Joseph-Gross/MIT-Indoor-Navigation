// THIS IS WHAT WE HAVE TO INTEGRATE INTO SRC probably



#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

TFT_eSPI tft = TFT_eSPI();

const uint16_t RESPONSE_TIMEOUT = 6000;
const uint16_t IN_BUFFER_SIZE = 3500;  // size of buffer to hold HTTP request
const uint16_t OUT_BUFFER_SIZE = 1000; // size of buffer to hold HTTP response
const uint16_t JSON_BODY_SIZE = 3000;
char request[IN_BUFFER_SIZE];
char response[OUT_BUFFER_SIZE]; // char array buffer to hold HTTP request
char json_body[JSON_BODY_SIZE];

/* CONSTANTS */
// Prefix to POST request:
const char PREFIX[] = "{\"wifiAccessPoints\": [";                 // beginning of json body
const char SUFFIX[] = "]}";                                       // suffix to POST request
const char API_KEY[] = "AIzaSyAQ9SzqkHhV-Gjv-71LohsypXUH447GWX8"; // don't change this and don't share this

const uint8_t BUTTON = 45;
const int MAX_APS = 5;

/* Global variables*/
uint8_t button_state;          // used for containing button state and detecting edges
uint8_t previous_button_state; // used for detecting button edges
uint32_t time_since_sample;    // used for microsecond timing

WiFiClientSecure client; // global WiFiClient Secure object
WiFiClient client2;      // global WiFiClient Secure object

uint8_t enter_dest_state;
uint8_t navigation_state;

const uint8_t STARTSCREEN = 0;
const uint8_t SELECT_DEST = 1;
const uint8_t CONFIRM_DEST = 2;
const uint8_t NAVIGATE = 3;

const uint8_t IDLE = 0;
const uint8_t ROUTING = 1;
const uint8_t FINDING_PATH = 2;
const uint8_t DISPLAY_DIRECTIONS = 3;

// choose one:

// const char NETWORK[] = "18_62";
// const char PASSWORD[] = "";
//
//
const char NETWORK[] = "MIT GUEST";
const char PASSWORD[] = "";
//
// const char NETWORK[] = "608_24G";
// const char PASSWORD[] = "608g2020";

/* Having network issues since there are 50 MIT and MIT_GUEST networks?. Do the following:
    When the access points are printed out at the start, find a particularly strong one that you're targeting.
    Let's say it is an MIT one and it has the following entry:
   . 4: MIT, Ch:1 (-51dBm)  4:95:E6:AE:DB:41
   Do the following...set the variable channel below to be the channel shown (1 in this example)
   and then copy the MAC address into the byte array below like shown.  Note the values are rendered in hexadecimal
   That is specified by putting a leading 0x in front of the number. We need to specify six pairs of hex values so:
   a 4 turns into a 0x04 (put a leading 0 if only one printed)
   a 95 becomes a 0x95, etc...
   see starting values below that match the example above. Change for your use:
   Finally where you connect to the network, comment out
     WiFi.begin(network, password);
   and uncomment out:
     WiFi.begin(network, password, channel, bssid);
   This will allow you target a specific router rather than a random one!
*/
uint8_t channel = 1;                                 // network channel on 2.4 GHz
byte bssid[] = {0x04, 0x95, 0xE6, 0xAE, 0xDB, 0x41}; // 6 byte MAC address of AP you're targeting.

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
int wifi_object_builder(char *object_string, uint32_t os_len, uint8_t channel, int signal_strength, uint8_t *mac_address)
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

char *SERVER = "googleapis.com"; // Server URL

char *SERVER2 = "608dev-2.net";

uint32_t timer;
uint32_t get_loc_timer;


/*------------------------------------------------------------------------------------------------------*/

void setup()
{
    Serial.begin(115200); // Set up serial port

    // SET UP SCREEN:
    tft.init();                            // init screen
    tft.setRotation(2);                    // adjust rotation
    tft.setTextSize(1);                    // default font size, change if you want
    tft.fillScreen(TFT_BLACK);             // fill background
    tft.setTextColor(TFT_PINK, TFT_BLACK); // set color of font to hot pink foreground, black background

    // SET UP BUTTON:
    delay(100); // wait a bit (100 ms)
    pinMode(BUTTON, INPUT_PULLUP);

    // PRINT OUT WIFI NETWORKS NEARBY
    int n = WiFi.scanNetworks();
    Serial.println("scan done");
    if (n == 0)
    {
        Serial.println("no networks found");
    }
    else
    {
        Serial.print(n);
        Serial.println(" networks found");
        for (int i = 0; i < n; ++i)
        {
            Serial.printf("%d: %s, Ch:%d (%ddBm) %s ", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "open" : "");
            uint8_t *cc = WiFi.BSSID(i);
            for (int k = 0; k < 6; k++)
            {
                Serial.print(*cc, HEX);
                if (k != 5)
                    Serial.print(":");
                cc++;
            }
            Serial.println("");
        }
    }
    delay(100); // wait a bit (100 ms)

    // if using regular connection use line below:
    WiFi.begin(NETWORK, PASSWORD);
    // if using channel/mac specification for crowded bands use the following:
    // WiFi.begin(network, password, channel, bssid);

    uint8_t count = 0; // count used for Wifi check times
    Serial.print("Attempting to connect to ");
    Serial.println(NETWORK);
    while (WiFi.status() != WL_CONNECTED && count < 12)
    {
        delay(500);
        Serial.print(".");
        count++;
    }
    delay(2000);
    if (WiFi.isConnected())
    { // if we connected then print our IP, Mac, and SSID we're on
        Serial.println("CONNECTED!");
        Serial.printf("%d:%d:%d:%d (%s) (%s)\n", WiFi.localIP()[3], WiFi.localIP()[2],
                      WiFi.localIP()[1], WiFi.localIP()[0],
                      WiFi.macAddress().c_str(), WiFi.SSID().c_str());
        delay(500);
    }
    else
    { // if we failed to connect just Try again.
        Serial.println("Failed to Connect :/  Going to restart");
        Serial.println(WiFi.status());
        ESP.restart(); // restart the ESP (proper way)
    }

    Wire.begin();
    Wire.setClock(400000);                            // 400 kbit/sec I2C speed
    // while (!Serial);                                  // required for Feather M4 Express
    // Serial.begin(115200);
    // ----- Set up the interrupt pin, its set as active high, push-pull
    pinMode(intPin, INPUT);
    digitalWrite(intPin, LOW);
    pinMode(myLed, OUTPUT);
    digitalWrite(myLed, HIGH);
    // ----- Display title
    Serial.println(F("MPU-9250 Quaternion Compass"));
    Serial.println("");
    delay(2000);
    // ----- Level surface message
    Serial.println(F("Place the compass on a level surface"));
    Serial.println("");
    delay(2000);

    // ----- Read the WHO_AM_I register, this is a good test of communication
    byte c = readByte(MPU9250_ADDRESS, WHO_AM_I_MPU9250);  // Read WHO_AM_I register for MPU-9250
    delay(1000);
    if ((c == 0x71) || (c == 0x73) || (c == 0x68)) // MPU9250=0x68; MPU9255=0x73
    {
      // Serial.println(F("MPU9250 is online..."));
      // Serial.println("");
      MPU9250SelfTest(SelfTest); // Start by performing self test and reporting values
      // Serial.println(F("Self test (14% acceptable)"));
      // Serial.print(F("x-axis acceleration trim within : ")); Serial.print(SelfTest[0], 1); Serial.println(F("% of factory value"));
      // Serial.print(F("y-axis acceleration trim within : ")); Serial.print(SelfTest[1], 1); Serial.println(F("% of factory value"));
      // Serial.print(F("z-axis acceleration trim within : ")); Serial.print(SelfTest[2], 1); Serial.println(F("% of factory value"));
      // Serial.print(F("x-axis gyration trim within : ")); Serial.print(SelfTest[3], 1); Serial.println(F("% of factory value"));
      // Serial.print(F("y-axis gyration trim within : ")); Serial.print(SelfTest[4], 1); Serial.println(F("% of factory value"));
      // Serial.print(F("z-axis gyration trim within : ")); Serial.print(SelfTest[5], 1); Serial.println(F("% of factory value"));
      // Serial.println("");
      calibrateMPU9250(gyroBias, accelBias); // Calibrate gyro and accelerometers, load biases in bias registers
      // Serial.println(F("MPU9250 accelerometer bias"));
      // Serial.print(F("x = ")); Serial.println((int)(1000 * accelBias[0]));
      // Serial.print(F("y = ")); Serial.println((int)(1000 * accelBias[1]));
      // Serial.print(F("z = ")); Serial.print((int)(1000 * accelBias[2]));
      // Serial.println(F(" mg"));
      // Serial.println("");
      // Serial.println(F("MPU9250 gyro bias"));
      // Serial.print(F("x = ")); Serial.println(gyroBias[0], 1);
      // Serial.print(F("y = ")); Serial.println(gyroBias[1], 1);
      // Serial.print(F("z = ")); Serial.print(gyroBias[2], 1);
      // Serial.println(F(" o/s"));
      // Serial.println("");
      // delay(1000);
      initMPU9250();
      // Serial.println(F("MPU9250 initialized for active data mode....")); // Initialize device for active mode read of acclerometer, gyroscope, and temperature
      // Serial.println("");
  

      // Read the WHO_AM_I register of the magnetometer, this is a good test of communication
      byte d = readByte(AK8963_ADDRESS, AK8963_WHO_AM_I);  // Read WHO_AM_I register for AK8963
      if (d == 0x48)
      {
        // ----- AK8963 found
        // Serial.println(F("AK8963 is online ..."));
        // Get magnetometer calibration from AK8963 ROM
        initAK8963(magCalibration);
      //   Serial.println(F("AK8963 initialized for active data mode ...")); // Initialize device for active mode read of magnetometer
      //   Serial.println("");
      //   Serial.println(F("Asahi sensitivity adjustment values"));
      //   Serial.print(F("ASAX = ")); Serial.println(magCalibration[0], 2);
      //   Serial.print(F("ASAY = ")); Serial.println(magCalibration[1], 2);
      //   Serial.print(F("ASAZ = ")); Serial.println(magCalibration[2], 2);
      //   Serial.println("");
      //   delay(1000);
      }

      else
      {
        // ----- AK8963 not found
        // Serial.print(F("AK8963 WHO_AM_I = ")); Serial.println(c, HEX);
        // Serial.println(F("I should be 0x48"));
        // Serial.print(F("Could not connect to AK8963: 0x"));
        // Serial.println(d, HEX);
        // Serial.println("");
        while (1) ; // Loop forever if communication doesn't happen

      }
    }
    else
    {
      // ----- MPU9250 not found
      Serial.print(F("MPU9250 WHO_AM_I = ")); Serial.println(c, HEX);
      Serial.println(F("I should be 0x71 or 0x73"));
      Serial.print(F("Could not connect to MPU9250: 0x"));
      Serial.println(c, HEX);
      Serial.println("");
      while (1) ; // Loop forever if communication doesn't happen
    }
    // --------------------------
    // TASK 0,1 tasks & messages
    // --------------------------
    if ((TASK == 0) || (TASK == 1))
    {
  #ifdef LCD2
      // tft.clear();
      tft.fillScreen(BACKGROUND);
      tft.setCursor(0, 0);
      tft.print(F("Tumble compass"));
      tft.setCursor(0, 1);
      tft.print(F("for 30 seconds"));
  #endif
      // ----- Calculate magnetometer offsets & scale-factors
      Serial.println(F("Magnetometer calibration ..."));
      Serial.println(F("Tumble/wave device for 30 seconds in a figure 8"));
      delay(2000);
      magCalMPU9250(magBias, magScale);

  #ifdef LCD2
      // tft.clear();
      tft.fillScreen(BACKGROUND);
      tft.setCursor(0, 0);
      tft.print(F("Stop tumbling"));
  #endif
      Serial.println(F("Stop tumbling"));
      Serial.println("");
      delay(4000);
    }
    if (TASK == 1)
    {
  #ifdef LCD2
      // tft.clear();
      tft.fillScreen(BACKGROUND);
      tft.setCursor(0, 0);
      tft.print(F("Record offsets"));
      tft.setCursor(0, 1);
      tft.print(F("& scale-factors"));
  #endif
      // ----- Message
      Serial.println(F("------------------------------------------"));
      Serial.println(F("Copy-&-paste the following code into your "));
      Serial.println(F("Arduino header then delete the old code."));
      Serial.println(F("------------------------------------------"));
      Serial.println(F(""));
      Serial.println(F("float"));
      Serial.print(F("Mag_x_offset = "));
      Serial.print(magBias[0]);
      Serial.println(",");
      Serial.print(F("Mag_y_offset = "));
      Serial.print(magBias[1]);
      Serial.println(",");
      Serial.print(F("Mag_z_offset = "));
      Serial.print(magBias[2]);
      Serial.println(",");
      Serial.print(F("Mag_x_scale = "));
      Serial.print(magScale[0]);
      Serial.println(",");
      Serial.print(F("Mag_y_scale = "));
      Serial.print(magScale[1]);
      Serial.println(",");
      Serial.print(F("Mag_z_scale = "));
      Serial.print(magScale[2]);
      Serial.println(F(";"));

      // ----- Halt program
      while (true);

    }

    timer = millis();
    distance = 255; // for testing
    direction_angle = 0; // for testing
    navigating = true;

    get_loc_timer = millis();
    enter_dest_state = STARTSCREEN;
}

void navigate_loop()
{
    if (millis() - get_loc_timer >= 5000)
    { // every 2 seconds, get the user's location
        int offset = sprintf(json_body, "%s", PREFIX);
        int n = WiFi.scanNetworks(); // run a new scan. could also modify to use original scan from setup so quicker (though older info)
        Serial.println("scan done");
        if (n == 0)
        {
            Serial.println("no networks found");
        }
        else
        {
            int max_aps = max(min(MAX_APS, n), 1);
            for (int i = 0; i < max_aps; ++i)
            {                                                                                                                             // for each valid access point
                uint8_t *mac = WiFi.BSSID(i);                                                                                             // get the MAC Address
                offset += wifi_object_builder(json_body + offset, JSON_BODY_SIZE - offset, WiFi.channel(i), WiFi.RSSI(i), WiFi.BSSID(i)); // generate the query
                if (i != max_aps - 1)
                {
                    offset += sprintf(json_body + offset, ","); // add comma between entries except trailing.
                }
            }
            sprintf(json_body + offset, "%s", SUFFIX);
            Serial.println(json_body);
            int len = strlen(json_body);
            // Make a HTTP request:
            Serial.println("SENDING REQUEST");
            request[0] = '\0'; // set 0th byte to null
            offset = 0;        // reset offset variable for sprintf-ing
            offset += sprintf(request + offset, "POST https://www.googleapis.com/geolocation/v1/geolocate?key=%s  HTTP/1.1\r\n", API_KEY);
            offset += sprintf(request + offset, "Host: googleapis.com\r\n");
            offset += sprintf(request + offset, "Content-Type: application/json\r\n");
            offset += sprintf(request + offset, "cache-control: no-cache\r\n");
            offset += sprintf(request + offset, "Content-Length: %d\r\n\r\n", len);
            offset += sprintf(request + offset, "%s\r\n", json_body);
            do_https_request(SERVER, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, false);
            Serial.println("-----------");
            Serial.println(response);
            Serial.println("-----------");
            // For Part Two of Lab04B, you should start working here. Create a DynamicJsonDoc of a reasonable size (few hundred bytes at least...)
        }

        char *ptr_1;
        ptr_1 = strchr(response, '{');
        // int first_bracket = ptr_1;

        char *ptr_2;
        ptr_2 = strrchr(response, '}');
        int last_bracket = ptr_2 - ptr_1 + 1;
        // char new_resp[OUT_BUFFER_SIZE];
        char destination[JSON_BODY_SIZE];
        strncpy(destination, ptr_1, last_bracket);

        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, destination);

        double latitude = doc["location"]["lat"];
        double longitude = doc["location"]["lng"];

        tft.setCursor(0, 10, 2);
        tft.printf("Current location:\n");
        tft.printf("Lat: %f\n", latitude);
        tft.printf("Lon: %f\n", longitude);

        request[0] = '\0';
        response[0] = '\0';

        longitude = -71.0926422;
        latitude = 42.3582736;

        // !!!! fix this!!!!
        sprintf(request, "GET https://608dev-2.net/sandbox/sc/team8/server_src/request_handler.py?user_id=1&lat=%lf&lon=%lf&destination=2&current_floor=0&destination_floor=0  HTTP/1.1\r\n", longitude, latitude);
        strcat(request, "Host: 608dev-2.net\r\n");
        strcat(request, "\r\n");

        do_http_request(SERVER2, request, response, OUT_BUFFER_SIZE, RESPONSE_TIMEOUT, true);

        Serial.print("response_from_python: ");
        Serial.println(response);


        ptr_1 = strchr(response, '{');
        // int first_bracket = ptr_1;

        ptr_2 = strrchr(response, '}');
        last_bracket = ptr_2-ptr_1+1;
        strncpy(destination, ptr_1, last_bracket);

        error = deserializeJson(doc, destination);

        Serial.print("dir_next_node: ");
        float dir_next_node = doc["dir_next_node"];
        Serial.println(dir_next_node);

        refresh_data();                              // This must be done each time through the loop
        calc_quaternion();                           // This must be done each time through the loop
        // ----- Processing Tasks

        switch (TASK) {
          case 2:
            compass_cal();                          // Get compass offsets and scale-factors using Processing "compass_cal.pde" (circle-method)
            break;
          case 3:
            compass_rose();                         // View compass heading using Processing "compass_rose.pde"
            break;
          default:
            break;
        }
        // ----- Perform these tasks every 500mS

        delt_t = millis() - count;
        if (delt_t > 500)
        {
          switch (TASK)
          {
            case 0:
              view_heading_LCD();                   // Send data to LCD display
              break;
            case 4:
              view_registers_SM();                  // Send  MPU9250 register contents to Serial Monitor (115200 bauds)
              break;
            case 5:
              view_heading_SM();                    // Send compass pitch roll & heading to Serial Monitor (115200 bauds)
              break;
            case 6:
              view_heading_LCD();                   // Send data to LCD display
              break;
            // case 20:
            //   angle_return();
            default:
              break;
          }
        
        // based on distance go hot to cold
        // need to make sure angle is an int from 0 to 359
        // need to make sure distance is an int from 0 to 255. SCALE APPROPRIATELY!
        if (abs(direction_angle-prev_direction_angle)>3 || abs(distance-prev_distance)>1){
          tft.fillScreen(BACKGROUND);
          direction_angle = -angle_return(); // function that gets yaw = heading
          arrow.update(navigating, direction_angle, distance, dir_next_node);
          tft.setCursor(0,0);
          tft.print("Angle is ");
          tft.print(-direction_angle);
        }
          // ----- Housekeeping
          // digitalWrite(myLed, !digitalRead(myLed));  // Toggle led
        count = millis(); 
    }
    get_loc_timer = millis();
}
}

// main body of code
void loop()
{
    button_state = digitalRead(BUTTON);

    switch (enter_dest_state)
    {
    case STARTSCREEN:
        tft.cursor(0, 10, 3);
        tft.fillScreen(TFT_BLACK);
        tft.printf("Input Valid Building"); // change building to classroom later (or give option of either classroom or building)

        // if (previous_button_state == 0 && button_state == 1){ // button just clicked
        //   enter_dest_state = SELECT_DEST;
        // }
        enter_dest_state = SELECT_DEST;
        break;
    case SELECT_DEST:
        // here's where wiki input selection will go
        if (previous_button_state == 0 && button_state == 1)
        { // button just clicked, now we will make sure that building is valid
            enter_dest_state = CONFIRM_DEST;
        }
        break;
    case CONFIRM_DEST:
        // check if building is valid, if yes, continue to navigation, if no, go back to STARTSCREEN to redisplay starting display
        if (true)
        { // right now, we are just gonna assume all inputs are a valid building
            enter_dest_state = NAVIGATE;
        }

        break;
    case NAVIGATE:
        // within here, we will have another state machine which handles actually navigating the user
        // once the user reaches their destination, we will change navigation_state back to IDLE
        // and enter_dest_state back to STARTSCREEN
        navigate_loop();
        break;
    }

    // bool in_navigate = true;
    // if(in_navigate == true){ // if we are currently navigating, run navigate loop
    //   navigate_loop();
    // }

    previous_button_state = button_state;
}
