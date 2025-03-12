
//NodeMCU--------------------------
#include <SimpleTimer.h>  //https://github.com/jfturcot/SimpleTimer
#include <SPIFFS.h>
#include <string>
#include <SPI.h>
#include <Wire.h>
#include <TimeLib.h> //not necessary
#include <esp_now.h>
#include <ArduinoJson.h>
#include <map>
#include <esp_task_wdt.h> //not necessary

//WiFi--------------------------
#include <WiFiClient.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <mDNS.h>
#include <HTTPClient.h>
#include <ElegantOTA.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
// #include <ESPAsyncHTTPUpdateServer.h>
// #include <WebServer.h>
// #include <HTTPUpdateServer.h>

//clock-----------------------------
#include <NTPClient.h>
#include <WiFiUdp.h>
//Adafruit-----------------------------
#include <Adafruit_GFX.h>          //https://github.com/adafruit/Adafruit-GFX-Library
#include <Adafruit_SSD1306.h>      //https://github.com/adafruit/Adafruit_SSD1306
#include <Adafruit_Fingerprint.h>  //https://github.com/adafruit/Adafruit-Fingerprint-Sensor-Library
#include <Adafruit_SH110X.h>
//files-----------------------------
#include "icons.h"

//************************************************************************
//Fingerprint scanner Pins
#define Finger_Rx 14  //D5
#define Finger_Tx 12  //D6
//Oled pins
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)

//************************************************************************
WiFiClient client;
SimpleTimer timer;
// SoftwareSerial mySerial(Finger_Rx, Finger_Tx);
HardwareSerial mySerial(1);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
Adafruit_SH1106G display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//************************************************************************
/* Set these to your desired credentials. */
// const char *ssid = "Next-Network";
// const char *password = "1458635297";
const char *myHostname = "esp8266";
const char *ssid;
const char *presharedkey;
const char *apname = "ESP8266 AP";
const char *apsharedkey;

const char *update_path = "/firmware";
const char *update_username = "admin";
const char *update_password = "admin";
std::string sketch_Version = "V1.ino";
std::string spiffs_Version = "V1.spiffs";

String device_token;
String IP_str;
String SUBNET_str;
String GATEWAY_str;
String DNS_str;
std::string DHCP;
String USER_str = "admin";
String PASSWORD_str = "admin";
String wifi_Mode;
String toggle;
int number_of_wifis;
IPAddress IP;
IPAddress SUBNET;
IPAddress GATEWAY;
IPAddress DNS;
/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 254, 0);

// HTTPUpdateServer httpUpdater;
// ESPAsyncHTTPUpdateServer httpUpdater;
AsyncWebServer server(80);

unsigned long ota_progress_millis = 0;

String URL = "http://192.168.3.13";

//************************************************************************
String getData, Link;
//************************************************************************
int FingerID = 0, t1, t2;  // The Fingerprint ID from the scanner
bool device_Mode = false;  // Default Mode Enrollment
bool firstConnect = false;
uint8_t id;
unsigned long previousMillis = 0;
unsigned long lastResetTime = millis();
const unsigned long resetInterval = 86400000;  // 24 hours

bool dataSended = false;
int global_id = 0;
int global_fingerID; //not necessary
String accessToken;
String refreshToken;

//File
File myFile;
char attendance_filename[] = "/attendance.txt";

//DNS
const byte DNS_PORT = 53;
DNSServer dnsServer;

//Time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
unsigned long lastSend1;
unsigned long lastSend;

const int16_t I2C_MASTER = 0x42;
const int16_t I2C_SLAVE = 0x08;

bool check_enroll = false;
String user_pk_global;

//------------------------ESPNOW
// uint8_t broadcastAddress1[] = { 0x84, 0xF3, 0xEB, 0xB4, 0x6B, 0xC3 };
uint8_t broadcastAddress1[] = { 0x48, 0xE7, 0x29, 0x54, 0xD2, 0x19 };
uint8_t broadcastAddress2[] = { 0x84, 0xF3, 0xEB, 0xB4, 0x57, 0x74 };
uint8_t broadcastAddress3[] = { 0x84, 0xF3, 0xEB, 0xB4, 0x70, 0x05 };
// uint8_t broadcastAddress4[] = { 0x84, 0xF3, 0xEB, 0xB4, 0x6B, 0x57 };
uint8_t broadcastAddress4[] = { 0x48, 0xE7, 0x29, 0x55, 0x8B, 0x72 };
uint8_t broadcastAddress5[] = { 0x98, 0xCD, 0xAC, 0x26, 0x22, 0x3A };
uint8_t broadcastAddress6[] = { 0x3C, 0x61, 0x05, 0xD1, 0xEE, 0x20 };
uint8_t broadcastAddress[6];

std::map<int, String> finger_closet;

esp_now_peer_info_t peerInfo;

typedef struct message {
  int id;
} message;
message myMessage;

const int ledPin = 2;
String ledState;
String processor(const String &var) {
  Serial.println(var);
  if (var == "STATE") {
    if (digitalRead(ledPin)) {
      ledState = "ON";
    } else {
      ledState = "OFF";
    }
    Serial.print(ledState);
    return ledState;
  }
  return String();
}


//************************************************************************
void setup() {

  WiFi.disconnect();
  Serial.begin(115200);
  if (!SPIFFS.begin(true)) {  // Format SPIFFS if mount fails
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //  if (SPIFFS.format()) Serial.println(" SPIFFS is Formatted");
  // Wire.begin(D2, D1, I2C_MASTER);

  delay(100);

  setOled();
  read_wifi_config();
  delay(2000);

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/iott.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/iott.jpg", "image/jpeg");
  });

  server.on("/WIFI.TXT", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Send the /WIFI.TXT file as the response
    // The third argument "application/json" specifies that the content type is JSON
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/WIFI.TXT", "application/json");

    // Add CORS headers to the response
    response->addHeader("Access-Control-Allow-Origin", "*");                    // Allow requests from any origin
    response->addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");  // Allowed HTTP methods
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");        // Allowed headers
    request->send(response);
  });

  server.on("/NET.TXT", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Send the /WIFI.TXT file as the response
    // The third argument "application/json" specifies that the content type is JSON
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/NET.TXT", "application/json");

    // Add CORS headers to the response
    response->addHeader("Access-Control-Allow-Origin", "*");                    // Allow requests from any origin
    response->addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");  // Allowed HTTP methods
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");        // Allowed headers
    request->send(response);
  });

  server.on("/AUTH.TXT", HTTP_POST, [](AsyncWebServerRequest *request) {
    // Send the /WIFI.TXT file as the response
    // The third argument "application/json" specifies that the content type is JSON
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/AUTH.TXT", "application/json");

    // Add CORS headers to the response
    response->addHeader("Access-Control-Allow-Origin", "*");                    // Allow requests from any origin
    response->addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");  // Allowed HTTP methods
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");        // Allowed headers
    request->send(response);
  });

  server.on("/write", writeMonitor);
  server.on("/read", readMonitor);
  server.on("/enroll", checkEnroll);
  server.on("/delete", checkDelete);
  server.on("/wificonfig", wifi_config);
  server.on("/networkconfig", network_config);
  server.on("/authconfig", auth_config);
  server.on("/command", Command);
  server.on("/scan", scan_wifi);
  server.on("/format", SPIFFS_format);
  server.on("/generate_204", loadFromSPIFFS);
  server.onNotFound(loadFromSPIFFS);

  ElegantOTA.setAuth(update_username, update_username);
  ElegantOTA.begin(&server);  // Start ElegantOTA
  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);

  server.begin();

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  setRealTimer();

  display.setCursor(0, 0);
  display.setTextSize(1);
  display.clearDisplay();
  display.println(("Sketch v :" + sketch_Version).c_str());
  display.println(("SPIFFS v :" + spiffs_Version).c_str());

  display.display();
  // delay(5000);

  setFingerSensor();

  setESPNOW();

  if (WiFi.isConnected()) {
    offline_attendance();
  }

  Serial.println("test now : " + arr_to_str(broadcastAddress1));
  //   {0x84, 0xF3, 0xEB, 0xB4, 0x6B, 0x57};
  finger_closet[1] = arr_to_str(broadcastAddress1);
  finger_closet[2] = arr_to_str(broadcastAddress2);
  finger_closet[3] = arr_to_str(broadcastAddress3);
  finger_closet[4] = arr_to_str(broadcastAddress4);
  finger_closet[5] = arr_to_str(broadcastAddress5);
  finger_closet[6] = arr_to_str(broadcastAddress6);

  // esp_task_wdt_init(10, true);
}

void enrolling_process() {
  check_enroll = false;

  uint8_t p = getFingerprintEnroll();
  if (p != FINGERPRINT_OK) return;

  ackEnroll(user_pk_global);
  Serial.println("sent ackk");
}

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}

String arr_to_str(uint8_t broadcastAddress[]) {
  String addressStr = "";
  for (int i = 0; i < 6; i++) {
    if (broadcastAddress[i] < 0x10) {
      addressStr += "0";  // Add leading zero for single hex digits
    }
    addressStr += String(broadcastAddress[i], HEX);  // Convert to hex and concatenate
    if (i != 5) {
      addressStr += ":";
    }
  }
  return addressStr;
}

void setESPNOW() {
  Serial.println("setting espnow");
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);

  // register peer
  peerInfo.channel = 6;
  peerInfo.encrypt = false;
  // register first peer
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 1");
    // return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 2");
    // return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress3, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 3");
    // return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress4, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 4");
    // return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress5, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 5");
    // return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress6, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer 6");
    // return;
  }

  int wifiChannel = WiFi.channel();
  Serial.print("Wi-Fi Channel: ");
  Serial.println(wifiChannel);
}

void stringToMACArray(const String &macStr, uint8_t mac[6]) {
  int values[6];

  // Parse the string using sscanf
  if (sscanf(macStr.c_str(), "%x:%x:%x:%x:%x:%x",
             &values[0], &values[1], &values[2],
             &values[3], &values[4], &values[5])
      == 6) {
    // Convert the parsed values into the uint8_t array
    for (int i = 0; i < 6; ++i) {
      mac[i] = (uint8_t)values[i];
    }
  }
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  char macStr[18];
  Serial.print("Packet to: ");
  // Copies the sender mac address to a string
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  displayScreen(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setRealTimer() {
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(12600);
}

void setOled() {
  if (!display.begin(0x3C, true)) {  // Address 0x3D for 128x64
    Serial.println(("SSD1306 allocation failed"));
  }
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(("Sketch v :" + sketch_Version).c_str());
  display.println(("SPIFFS v :" + spiffs_Version).c_str());

  display.display();
  delay(5000);  // Pause for 2 seconds
  display.clearDisplay();
}

void setSpiffConfig() {
  if (!SPIFFS.begin()) {
    Serial.println(("Faild!!check SPIFFS"));
    display.print("Faild!!check SPIFFS");
    display.display();
    delay(60000);
    ESP.restart();
  }
  // Serial.println("SPIFFS OK");
  display.print("SPIFFS OK");
  display.display();
  delay(1000);
}

void read_wifi_config() {
  String config_str;

  displayConnecting();
  ////////////////////////////////////////////////////////////////////////////////read wifi config
  myFile = SPIFFS.open("/WIFI.TXT", "r");  ///open config.txt file
  config_str = myFile.readString();
  StaticJsonDocument<200> jsonBuffer1;
  DeserializationError error1 = deserializeJson(jsonBuffer1, config_str);
  if (error1) {
    Serial.println(("Failed to parse wifi file.\n Start ESP in AP Mode"));
    display.clearDisplay();
    //  display.setFont(ArialMT_Plain_10);
    display.print(("Failed to parse wifi file.\n Start ESP in AP Mode"));
    display.display();
    delay(1000);
    wifi_Mode = "WIFI_AP";
    //    wifi_Mode="WIFI_STA";

    run_wifi();
    return;
  }
  //////////////////////////////////////put json objects to String variables/////////
  wifi_Mode = jsonBuffer1["wifimode"].as<String>();
  ssid = jsonBuffer1["SSID"];
  presharedkey = jsonBuffer1["presharedkey"];
  apname = jsonBuffer1["APname"];
  apsharedkey = jsonBuffer1["APsharedkey"];

  myFile.close();

  //////////////////////////////////////////////////////////////////////////////read network config
  myFile = SPIFFS.open("/NET.TXT", "r");  ///open config.txt file
  config_str = myFile.readString();
  DynamicJsonDocument jsonBuffer2(256);
  DeserializationError error2 = deserializeJson(jsonBuffer2, config_str);
  if (error2) {
    display.clearDisplay();
    //  display.setFont(ArialMT_Plain_10);
    display.print("Failed to parse Network file.\n Start ESP in AP Mode");
    display.display();
    delay(1000);
    wifi_Mode = "WIFI_AP";
    run_wifi();
    return;
  }
  //////////////////////////////////////put json objects to String variables/////////
  DHCP = jsonBuffer2["DHCP"].as<std::string>();
  IP_str = jsonBuffer2["IP"].as<String>();
  SUBNET_str = jsonBuffer2["SUBNET"].as<String>();
  GATEWAY_str = jsonBuffer2["GATEWAY"].as<String>();
  DNS_str = jsonBuffer2["DNS"].as<String>();
  myFile.close();
  //////////////////////////////////////////////////////////////////////////////read auth config
  myFile = SPIFFS.open("/AUTH.TXT", "r");  ///open config.txt file
  config_str = myFile.readString();
  DynamicJsonDocument jsonBuffer3(256);
  DeserializationError error3 = deserializeJson(jsonBuffer3, config_str);
  if (error3) {
    display.clearDisplay();
    //  display.setFont(ArialMT_Plain_10);
    display.print("Failed to parse Authentication file.\n Start ESP in AP Mode");
    display.display();
    delay(1000);
    wifi_Mode = "WIFI_AP";

    run_wifi();
    return;
  }
  //////////////////////////////////////put json objects to String variables/////////
  USER_str = jsonBuffer3["USER"].as<String>();
  PASSWORD_str = jsonBuffer3["PASSWORD"].as<String>();
  myFile.close();

  run_wifi();

  displayConnected();
}

void displayConnecting() {
  display.clearDisplay();
  display.setTextSize(1);       // Normal 1:1 pixel scale
  display.setTextColor(WHITE);  // Draw white text
  display.setCursor(0, 0);      // Start at top-left corner
  display.print(("Connecting to \n"));
  display.setCursor(0, 50);
  display.setTextSize(2);
  display.print(ssid);
  display.drawBitmap(73, 10, Wifi_start_bits, Wifi_start_width, Wifi_start_height, WHITE);
  display.display();
}

void displayConnected() {
  display.clearDisplay();
  display.setTextSize(2);       // Normal 1:1 pixel scale
  display.setTextColor(WHITE);  // Draw white text
  display.setCursor(8, 0);      // Start at top-left corner
  display.print(("Connected \n"));
  display.drawBitmap(33, 15, Wifi_connected_bits, Wifi_connected_width, Wifi_connected_height, WHITE);
  display.display();
}

void setFingerSensor() {
  Serial.println("setFinger func");
  finger.begin(57600);
  Serial.println("setFinger func2");
  mySerial.begin(57600, SERIAL_8N1, 17, 16);
  Serial.println(("\n\nAdafruit finger detect test"));

  if (finger.verifyPassword()) {
    Serial.println(("Found fingerprint sensor!"));
    display.clearDisplay();
    display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else {
    Serial.println(("Did not find fingerprint sensor :("));
    display.clearDisplay();
    display.drawBitmap(32, 0, FinPr_failed_bits, FinPr_failed_width, FinPr_failed_height, WHITE);
    display.display();
    // while (1) { delay(1); }
  }
  //---------------------------------------------
  finger.getTemplateCount();
  // Serial.print(F("Sensor contains " + finger.templateCount + " templates"));
  Serial.println(("Waiting for valid finger..."));
}

//************************************************************************
void loop() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  if (millis() - lastResetTime >= resetInterval) {
    ESP.restart();  // Reset the ESP32 every 24 hours
  }

    .loop();
  timer.run();  
  dnsServer.processNextRequest();
  // server.handleClient();
  // ftpSrv.handleFTP();
  //check if there's a connection to Wi-Fi or not
  if (WiFi.status() != WL_CONNECTED && (millis() - previousMillis >= 20000)) {
    Serial.println("wifi disconnected");
    previousMillis = millis();
    // WiFi.disconnect();
    read_wifi_config();
    // run_wifi();
    // WiFi.reconnect();
    delay(3000);
    if (WiFi.isConnected()) {
      Serial.println("offline_attendance sending to server");

      offline_attendance();
    }
  }
  // checkServer();
  CheckFingerprint("");  //Check the sensor if the there a finger.
  delay(100);

  check_command_test();

  if (check_enroll) enrolling_process();
}

void offline_attendance() {
  myFile = SPIFFS.open("/attendance.txt", "r");
  if (!myFile) {
    Serial.println(("file open failed"));
    return;
  }
  while (myFile.available()) {
    Serial.println("attendanee reading");
    String finger_id = myFile.readStringUntil('\n');
    // Serial.println("finger_id is: " + finger_id);
    String time = myFile.readStringUntil('\n');
    // Serial.println("time is: " + time);
    displayScreen("finger_id is: " + finger_id + "\n time is: " + time);
    time.remove(time.length() - 1);
    ackOpenDoor(finger_id.toInt(), false, time);
    delay(2000);
  }
  myFile.close();
  remove_file();
}



//************************************************************************
void SPIFFS_format(AsyncWebServerRequest *request) {
  // if (!request->authenticate(USER_str.c_str(), PASSWORD_str.c_str()))
  //   return request->requestAuthentication();
  display.clearDisplay();
  // display.setFont(ArialMT_Plain_10);
  display.print("Formating SPIFFS\n Please Wait....");
  display.display();
  if (SPIFFS.format()) {
    Serial.println(("SPIFFS Formated"));
    display.clearDisplay();
    display.print("Formating SPIFF Finished");
    display.display();
    request->send(200, "text/plain", "SPIFFS Formated!!");
    // delay(2000);
  }
}

void scan_wifi(AsyncWebServerRequest *request) {
  // if (!request->authenticate(USER_str.c_str(), PASSWORD_str.c_str()))
  //   return request->requestAuthentication();
  display.clearDisplay();
  //  display.setFont(ArialMT_Plain_10);
  display.print("Scanning WiFi Stations");
  display.display();

  // WiFi.scanNetworks will return the number of networks found
  number_of_wifis = WiFi.scanNetworks();
  // Serial.println("scan done");
  if (number_of_wifis == 0) {
    display.print("No WiFi Stations found");
    display.display();
  } else {
    // Serial.print(number_of_wifis);
    display.print(String(number_of_wifis) + " WiFi Stations found");
    display.display();
    delay(1000);
    for (int i = 0; i < number_of_wifis; ++i) {
      // Print SSID and RSSI for each network found
      // Serial.print(i + 1);
      // Serial.print(": ");
      // Serial.print(WiFi.SSID(i));
      // Serial.print(" (");
      long rssi = WiFi.RSSI(i);
      rssi = 100 + rssi;
      // Serial.print(rssi);
      // Serial.print(") EncryptionType : ");
      byte type_wifi = WiFi.encryptionType(i);
      String Type_wifi_str;
      if (type_wifi == 2) Type_wifi_str = "TKIP (WPA)";
      if (type_wifi == 5) Type_wifi_str = "WEP";
      if (type_wifi == 4) Type_wifi_str = "AES (WPA)";
      if (type_wifi == 7) Type_wifi_str = "NONE";
      if (type_wifi == 8) Type_wifi_str = "AUTO";
      // Serial.println(Type_wifi_str);
      delay(10);
    }
    ///////////////////////////////////////////////////json object making stations array and send it to server////////////////////////
    DynamicJsonDocument jsonBuffer(256);
    // JsonObject json = jsonBuffer.createObject();
    JsonArray stations = jsonBuffer.createNestedArray("stations");
    for (int i = 0; i < number_of_wifis; ++i) {
      stations.add(WiFi.SSID(i));
    }
    String stationObj = "";
    serializeJson(jsonBuffer, stationObj);

    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", stationObj);

    // Add CORS headers to the response
    response->addHeader("Access-Control-Allow-Origin", "*");                    // Allow requests from any origin
    response->addHeader("Access-Control-Allow-Methods", "POST, GET, OPTIONS");  // Allowed HTTP methods
    response->addHeader("Access-Control-Allow-Headers", "Content-Type");        // Allowed headers
    request->send(response);
  }
}

void run_wifi() {

  //////string to IP conversion////
  String_to_IP(IP_str, IP);
  String_to_IP(SUBNET_str, SUBNET);
  String_to_IP(GATEWAY_str, GATEWAY);
  String_to_IP(DNS_str, DNS);
  ///////////////////////////////

  if (wifi_Mode == "WIFI_STA") {
    WiFi.mode(WIFI_STA);
    Run_Station();
  }
  if (wifi_Mode == "WIFI_AP") {
    WiFi.mode(WIFI_AP);
    Run_Accesspoint();
  }
  if (wifi_Mode == "WIFI_AP_STA") {
    WiFi.mode(WIFI_AP_STA);
    Run_Multi();
  }
}

void Run_Station() {

  Serial.println(" ");
  Serial.println(("Station mode\n Connecting to\n ") + String(ssid));
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  //  display.setFont(ArialMT_Plain_16);
  display.print(("Station mode\n Connecting to ") + String(ssid));
  display.display();
  delay(1000);
  if (ssid != "") {
    if (presharedkey != "") {
      WiFi.begin(ssid, presharedkey);  //////connecting to wifi
      display.print(".");
      display.display();
    } else {
      WiFi.begin(ssid);
    }
  } else {
    if (WiFi.SSID()) {
      Serial.println(("Running \n Last saved \n WiFi"));

      display.clearDisplay();
      //  display.setFont(ArialMT_Plain_16);
      display.print("Running \n Last saved \n WiFi");
      display.display();
      delay(1000);
      WiFi.begin();
    }
  }



  if (DHCP != "on") WiFi.config(IP, GATEWAY, SUBNET, DNS);  //if DHCP is no then set Static IP for ESP///
                                                            // Wait for connection
  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 30) {  //wait 15 seconds
    display.print(">");
    display.display();
    delay(500);
  }

  if (i == 31) {
    display.clearDisplay();
    display.print("Couldn't\n Connect to\n " + String(ssid));
    Serial.println("Couldn't\n Connect to\n " + String(ssid));
    display.display();
    delay(2000);
    Serial.println(ssid);
    delay(500);
    wifi_Mode = "WIFI_AP";
    run_wifi();
    return;
  }

  //  // Setup MDNS responder
  //   if (!MDNS.begin(myHostname)) {
  //     Serial.println("Error setting up MDNS responder!");
  //   } else {
  //     Serial.println("mDNS responder started");
  //     // Add service to MDNS-SD
  //     MDNS.addService("http", "tcp", 80);
  //   }
  //////////////////////////////////////////////check for Update all sketch  or spiffs from  external server ////////

  //  IPs = WiFi.localIP().toString();

  show_IPs();
}

void show_IPs() {

  String MAC = WiFi.macAddress();
  device_token = MAC;
  // device_token = MAC;
  String IPs = WiFi.localIP().toString();
  String GATEWAYs = WiFi.gatewayIP().toString();
  String SUBNETs = WiFi.subnetMask().toString();
  String DNSs = WiFi.dnsIP().toString();

  display.clearDisplay();
  display.setTextSize(1);              // Normal 2:2 pixel scale
  display.setTextColor(SH110X_WHITE);  // Draw white text
  display.setCursor(0, 0);             // Start at top-left corner
  // display.drawChar(0,0,"MAC : "+WiFi.macAddress(),WHITE, BLACK, 2);
  display.println(("MAC : ") + MAC);
  display.println(("IP Address:") + IPs);
  display.println(("Subnet: ") + SUBNETs);
  display.println(("Gateway :") + GATEWAYs);
  display.println(("DNS IP: ") + DNSs);
  display.println(("DHCP: " + DHCP).c_str());
  display.display();

  Serial.println("MAC : " + WiFi.macAddress());
  Serial.println("IP Address : " + IPs);
  Serial.println("Subnet : " + GATEWAYs);
  Serial.println("DNS IP : " + DNSs);
  Serial.println(("DHCP : " + DHCP).c_str());

  delay(3000);
}

void Run_Accesspoint() {
  Serial.println("Starting\n Access point " + String(apname));

  display.clearDisplay();
  //  display.setFont(ArialMT_Plain_16);
  display.print("Starting\n Access point\n" + String(apname));
  display.display();
  delay(2000);
  if (apsharedkey != NULL) {
    WiFi.softAP(apname, apsharedkey);  //password option
    delay(500);
  } else {
    WiFi.softAP(apname);
    delay(500);
  }
  Serial.print("ip is hhere ");
  // Serial.println(WiFi.localIP());
}

void Run_Multi() {
  Run_Accesspoint();
  Run_Station();
}

void String_to_IP(String str, IPAddress &IP_addr) {
  int c1 = str.indexOf('.');          //first place to cut string
  int c2 = str.indexOf('.', c1 + 1);  //second place
  int c3 = str.indexOf('.', c2 + 1);  //Third place
  int ln = str.length();              //last place to stop
  IP_addr[0] = str.substring(0, c1).toInt();
  IP_addr[1] = str.substring(c1 + 1, c2).toInt();
  IP_addr[2] = str.substring(c2 + 1, c3).toInt();
  IP_addr[3] = str.substring(c3 + 1, ln).toInt();
}

void loadFromSPIFFS(AsyncWebServerRequest *request) {
  Serial.println("page is being loaad ...");
  String path = request->url();

  String dataType = String();
  if (path.endsWith("/")) path += "index.htm";
  if (path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if (path.endsWith(".htm")) dataType = "text/html";
  else if (path.endsWith(".css")) dataType = "text/css";
  else if (path.endsWith(".js")) dataType = "text/javascript";
  else if (path.endsWith(".png")) dataType = "image/png";
  else if (path.endsWith(".gif")) dataType = "image/gif";
  else if (path.endsWith(".jpg")) dataType = "image/jpeg";
  else if (path.endsWith(".ico")) dataType = "image/x-icon";
  else if (path.endsWith(".xml")) dataType = "text/xml";
  else if (path.endsWith(".pdf")) dataType = "application/pdf";
  else if (path.endsWith(".zip")) dataType = "application/zip";

  // if (!request->authenticate(USER_str.c_str(), PASSWORD_str.c_str()))
  //   return request->requestAuthentication();

  request->send(SPIFFS, path, dataType, false, processor);

  Serial.println("page is loaded !");

  String url = request->url();
  Serial.println(url);
}

////////////////////////////////////////////wifi conf from Web route
void wifi_config(AsyncWebServerRequest *request) {

  wifi_Mode = request->arg("Wifimode");
  String SSID_str = request->arg("SSID");
  String preshared_str = request->arg("presharedkey");
  String APname_str = request->arg("APname");
  String APsharedkey_str = request->arg("APsharedkey");
  DynamicJsonDocument jsonBuffer(256);
  jsonBuffer["wifimode"] = wifi_Mode;
  jsonBuffer["SSID"] = SSID_str;
  jsonBuffer["presharedkey"] = preshared_str;
  jsonBuffer["APname"] = APname_str;
  jsonBuffer["APsharedkey"] = APsharedkey_str;

  myFile = SPIFFS.open("/WIFI.TXT", "w");
  if (!myFile) {
    Serial.println("Failed to open config file ");
    return;
  }
  serializeJson(jsonBuffer, myFile);

  read_SPIFFS("/WIFI.TXT");

  myFile.close();
  request->send(200, "text/plain", "WiFi settings Saved");
}

void read_SPIFFS(String fn) {

  myFile = SPIFFS.open(fn, "r");
  if (!myFile) Serial.println("file open failed");
  while (myFile.available()) {
    Serial.write(myFile.read());
  }
  myFile.close();
}

/////////////////////////////////network config web from route
void network_config(AsyncWebServerRequest *request) {
  // if (!request->authenticate(USER_str.c_str(), PASSWORD_str.c_str()))
  //   return request->requestAuthentication();
  DHCP = (request->arg("DHCP")).c_str();
  IP_str = request->arg("IP");
  SUBNET_str = request->arg("SUBNET");
  GATEWAY_str = request->arg("GATEWAY");
  DNS_str = request->arg("DNS");
  DynamicJsonDocument jsonBuffer(256);
  jsonBuffer["DHCP"] = DHCP;
  jsonBuffer["IP"] = IP_str;
  jsonBuffer["SUBNET"] = SUBNET_str;
  jsonBuffer["GATEWAY"] = GATEWAY_str;
  jsonBuffer["DNS"] = DNS_str;

  myFile = SPIFFS.open("/NET.TXT", "w");
  if (!myFile) {
    Serial.println("Failed to open config file ");
    return;
  }
  serializeJson(jsonBuffer, myFile);

  myFile.close();
  request->send(200, "text/plain", "Network settings Saved");
}

///////////////////////auth config from web route
void auth_config(AsyncWebServerRequest *request) {
  // if (!request->authenticate(USER_str.c_str(), PASSWORD_str.c_str()))
  //   return request->requestAuthentication();
  USER_str = request->arg("USER");
  PASSWORD_str = request->arg("PASSWORD");
  DynamicJsonDocument jsonBuffer(256);
  jsonBuffer["USER"] = USER_str;
  jsonBuffer["PASSWORD"] = PASSWORD_str;
  myFile = SPIFFS.open("/AUTH.TXT", "w");
  if (!myFile) {
    Serial.println("Failed to open config file ");
    return;
  }
  serializeJson(jsonBuffer, myFile);

  myFile.close();
  request->send(200, "text/plain", "New User and Password Saved ");
}

/////////////////////////////comand from web route
void Command(AsyncWebServerRequest *request) {
  if (request->arg("ESP") == "default") {  ////////deafault factory command/////

    /////////////////////default WIFI config//////////
    DynamicJsonDocument jsonBuffer1(256);
    jsonBuffer1["wifimode"] = "WIFI_AP";
    jsonBuffer1["SSID"] = "";
    jsonBuffer1["presharedkey"] = "";
    jsonBuffer1["APname"] = "ESP8266 AP";
    jsonBuffer1["APsharedkey"] = "12345678";

    JsonArray stations = jsonBuffer1.createNestedArray("stations");
    for (int i = 0; i < number_of_wifis; ++i) {
      stations.add(WiFi.SSID(i));
    }
    myFile = SPIFFS.open("/WIFI.TXT", "w");
    if (!myFile) {
      Serial.println("Failed to open config file ");
      return;
    }
    serializeJson(jsonBuffer1, myFile);
    myFile.close();
    //////////////////////////////////////default Network config///
    DynamicJsonDocument jsonBuffer2(256);
    jsonBuffer2["DHCP"] = "on";
    jsonBuffer2["IP"] = "";
    jsonBuffer2["SUBNET"] = "";
    jsonBuffer2["GATEWAY"] = "";
    jsonBuffer2["DNS"] = "";
    myFile = SPIFFS.open("/NET.TXT", "w");
    if (!myFile) {
      Serial.println("Failed to open config file ");
      return;
    }
    serializeJson(jsonBuffer2, myFile);
    myFile.close();
    /////////////////////////////////////////default auth config
    DynamicJsonDocument jsonBuffer3(256);
    jsonBuffer3["USER"] = "admin";
    jsonBuffer3["PASSWORD"] = "admin";
    myFile = SPIFFS.open("/AUTH.TXT", "w");
    if (!myFile) {
      Serial.println("Failed to open config file ");
      return;
    }
    serializeJson(jsonBuffer3, myFile);
    myFile.close();
    request->send(200, "text/plain", "All Settings changed to Default and esp rebooted in AP mode .for setup please connect to ESP8266 AP and AP shared key is : 12345678");
    ////////////////////////////////////////////
    delay(3000);
    ESP.restart();
  }
  if (request->arg("ESP") == "reboot") {  //////reboot comand////////
    request->send(200, "text/plain", "ESP Rebooted");
    delay(3000);
    ESP.restart();
  }
}
//**************************update skech and spiff**************************

//************************************************************************
uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available())
      ;
    num = Serial.parseInt();
  }
  return num;
}

void check_command_test() {
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim();
    if (command == "time") {
      Serial.println(time_format());
    }
    if (command == "en") {
      Serial.println("Enter the ID to enroll:");

      int id = readnumber();
      global_id = id;
      if (id >= 0) {
        // sendEnrollTest(String(id));
        getFingerprintEnroll();
        // deleteFingerprint(id);
      } else {
        Serial.println("Invalid ID. Please enter a positive integer.");
      }
    }
    if (command == "del") {
      Serial.println("Enter the ID to delete:");

      int id = readnumber();
      // Serial.print("id is: ");
      // Serial.println(id);
      if (id >= 0) {
        deleteFingerprint(id);
      } else {
        Serial.println("Invalid ID. Please enter a positive integer.");
      }
    }
    if (command == "s") {
      // Serial.println("Enter the fingerID to send:");

      // int id = readnumber();
      SendFingerSlave(1);
    }
    if (command == "r") {
      // Serial.println("Enter the ID to send:");

      // int id = readnumber();
      // Serial.print("id is: ");
      // Serial.println(id);
      // if (id > 0) {
      // uint8_t fingerTemplate[512];
      // String base64Template="ywkeTJ5r+nTFhn5gzO2CfKmbDXi5thV0k2gdYLPuJZyv+Vnx6WovHsTEGwTyb8s4C5TlMlS0Z30iYAxUC84vL4h5GYDZqfouzMN/Hhlj3X65iroY1ZRSgjf405D4r7+Bvxu+G3ruvtsjVaO3ggkxA2LAb2eYGFUbg9nSJ794Y/pAZO8B/////wIAglsewJ0Ju98dcOzl3cQ1JIzGKHg3zSWQq+3smAwLCaQhDExYwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABFx7wH/////AgCCywkXUJ5r+nTFhn5gzO2CfKmbDXi5thV0k2wdYLPuJZyn+Vpx52u6APDEIwW8b7cHVp7NH0S0/31daNRmC87FL9/Z2ZXCqz+nxWXZpvJg7va3iJKCIJgaXLcwvz57PpPBDZ3ms77gcNpR3DD2O1axW4Pi4VtFpek/P7vWvuR2X0NGM+8B/////wgAgssJHkyea/p0xYZ+YMztgnypmw14ubYVdJNoHWCz7iWcr/lZ8elqLx7ExBsE8m/LOAuU5TJUtGd9ImAMVAvOLy+IeRmA2an6LszDfx4ZY91+uYq6GNWUUoI3+NOQ+K+/gb8bvht67r7bI1U=";

      // memset(fingerTemplate, 0xff, 512); // clear the buffer

      // Decode the Base64 template
      // int decodedLength = base64::decode(fingerTemplate, base64Template.c_str(), base64Template.length());
      // fingerTemplate = base64::decode(base64Template, sizeof(base64Template));
      // int decodedLength = Base64.decodedLength(base64Template, sizeof(base64Template));
      // char decodedString[decodedLength + 1];
      // Base64.decode(decodedString, base64Template, sizeof(base64Template));

      // const char base64Template[] = "ywkeTJ5r+nTFhn5gzO2CfKmbDXi5thV0k2gdYLPuJZyv+Vnx6WovHsTEGwTyb8s4C5TlMlS0Z30iYAxUC84vL4h5GYDZqfouzMN/Hhlj3X65iroY1ZRSgjf405D4r7+Bvxu+G3ruvtsjVaO3ggkxA2LAb2eYGFUbg9nSJ794Y/pAZO8B/////wIAglsewJ0Ju98dcOzl3cQ1JIzGKHg3zSWQq+3smAwLCaQhDExYwAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAABFx7wH/////AgCCywkXUJ5r+nTFhn5gzO2CfKmbDXi5thV0k2wdYLPuJZyn+Vpx52u6APDEIwW8b7cHVp7NH0S0/31daNRmC87FL9/Z2ZXCqz+nxWXZpvJg7va3iJKCIJgaXLcwvz57PpPBDZ3ms77gcNpR3DD2O1axW4Pi4VtFpek/P7vWvuR2X0NGM+8B/////wgAgssJHkyea/p0xYZ+YMztgnypmw14ubYVdJNoHWCz7iWcr/lZ8elqLx7ExBsE8m/LOAuU5TJUtGd9ImAMVAvOLy+IeRmA2an6LszDfx4ZY91+uYq6GNWUUoI3+NOQ+K+/gb8bvht67r7bI1U=";
      // int decodedLength = base64::decodeLength(base64Template);
      // uint8_t output[decodedLength];
      // base64::decode(base64Template, output);

      // Add the fingerprint template to the sensor at ID 1
      // addFingerprintTemplate(output, decodedLength, 1);
      // downloadAndSendFingerprintTemplate();
      // } else {
      //     Serial.println("Invalid ID. Please enter a positive integer.");
      // }
    }
    if (command == "format") {
      Serial.println("Formatting SPIFFS please Wait ....");
      if (SPIFFS.format()) Serial.println(" SPIFFS is Formatted");
    }
    if (command == "remove") {
      remove_file();
    }
    if (command == "read") {
      Serial.println("reading file!!!!");

      myFile = SPIFFS.open(attendance_filename, "r");
      if (!myFile) Serial.println("file open failed");

      while (myFile.available()) {

        String time = myFile.readStringUntil('\n');
        Serial.println("time is: " + time);
        String finger_id = myFile.readStringUntil('\n');
        Serial.println("finger_id is: " + finger_id);
        time.remove(time.length() - 1);
        SendFingerprint(finger_id.toInt(), time);

        delay(500);
      }
      myFile.close();
    }
    if (command == "write") {
      // myFile = SPIFFS.open(attendance_filename, "a");
      // myFile.println(1);
      // String timeNow = time_format();
      // Serial.println(timeNow);
      // displayScreen(timeNow);
      // delay(2000);
      // myFile.println(timeNow);

      // myFile.close();

      // Serial.println("atendance is written");
    }
    if (command == "rr") {
      myFile = SPIFFS.open(attendance_filename, "r");
      if (!myFile) Serial.println("file open failed");
      int str[20];
      while (myFile.available()) {
        // Serial.write(myFile.read());
        // int i = 0;
        // str[i] = myFile.read();
        // i++;
        String inp = myFile.readString();

        int newlineIndex = inp.indexOf('\n');
        String firstPart = inp.substring(0, newlineIndex);
        String secondPart = inp.substring(newlineIndex + 1);

        // Print the separated strings
        Serial.print("First Part: ");
        Serial.println(firstPart);
        Serial.print("Second Part: ");
        Serial.println(secondPart);

        myFile.close();
        remove_file();
      }

      // String numberString = "";  // A String object to hold the concatenated result

      // for (int i = 0; i < 20; i++) {
      //   numberString += String(str[i]);
      //   numberString += " ";

      // }
      // Serial.println(numberString);

      myFile.close();
    }
  }
}



void remove_file() {
  if (SPIFFS.exists(attendance_filename)) {
    SPIFFS.remove(attendance_filename);
    Serial.print(attendance_filename);
    Serial.println(" Removed Succesfuly");
  } else {
    Serial.println(attendance_filename);
    Serial.println(" Does not exist");
  }
}

String time_format() {
  String time = timeClient.getFormattedDate();

  return time;
}

//***********************

void CheckFingerprint(String send_time) {
  // If there no fingerprint has been scanned return -1 or -2 if there an error or 0 if there nothing, The ID start form 1 to 162
  // Get the Fingerprint ID from the Scanner
  FingerID = getFingerprintID();
  DisplayFingerprintID();
  if (FingerID > 0) {
    SendFingerSlave(FingerID);
  }
}

//************Display the fingerprint ID state on the OLED*************
bool DisplayFingerprintID() {
  //Fingerprint has been detected
  if (FingerID > 0 && dataSended == false) {
    display.clearDisplay();
    display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
    // SendEnrollFingerprint( FingerID );
    delay(200);
    dataSended = true;
    return true;
  }
  //---------------------------------------------
  //No finger detected
  else if (FingerID == 0) {
    display.clearDisplay();
    display.drawBitmap(32, 0, FinPr_start_bits, FinPr_start_width, FinPr_start_height, WHITE);
    display.display();
  }
  //---------------------------------------------
  //Didn't find a match
  else if (FingerID == -1) {
    display.clearDisplay();
    display.drawBitmap(34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
    display.display();
  }
  //---------------------------------------------
  //Didn't find the scanner or there an error
  else if (FingerID == -2) {
    display.clearDisplay();
    display.drawBitmap(32, 0, FinPr_failed_bits, FinPr_failed_width, FinPr_failed_height, WHITE);
    display.display();
  }
  return false;
}

// String GetDoorIP(int finger_id) {
//   // String door_ip;

//   String macStr = finger_closet[finger_id];
//   Serial.println("mac is local: " + macStr);

//   display.clearDisplay();
//   display.setTextSize(1);    // Normal 2:2 pixel scale
//   display.setCursor(15, 0);  // Start at top-left corner
//   display.print(("Welcome"));
//   display.setCursor(0, 10);
//   display.print("door_mac = ");
//   display.print(macStr);

//   display.display();

//   return macStr;
// }

void SendFingerSlave(int finger_id) {
  global_fingerID = finger_id;
  String macStr = finger_closet[finger_id];
  Serial.println("mac finger: " + macStr);

  uint8_t mac[6];
  stringToMACArray(macStr, mac);

  myMessage.id = 11;
  esp_err_t result = esp_now_send(mac, (uint8_t *)&myMessage, sizeof(myMessage));

  if (result == ESP_OK) {
    Serial.println("Sent with success");
    ackOpenDoor(finger_id, true, "");
  } else {
    Serial.println("Error sending the data");
    displayScreen("Error sending the data");
    ackOpenDoor(finger_id, false, "");
  }
  delay(2000);
}

void ackOpenDoor(int finger_id, bool ack, String send_time) {
  getAccessToken();
  if (WiFi.isConnected() && accessToken != "") {
    Serial.println("ackOpenDoor func");
    Serial.println("finger_id: " + finger_id);
    Serial.println(ack);
    HTTPClient http;  //Declare object of class HTTPClient
    //GET Data
    String url = URL + "/accounts/api/v1/user/fingerclosetdoor/ack/";
    http.begin(client, url);  //initiate HTTP request   //Specify content-type header
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + accessToken);

    // JSON data
    StaticJsonDocument<200> doc;
    doc["finger_id"] = finger_id;
    doc["mac_id"] = device_token;
    doc["ack"] = ack;
    if (send_time != NULL) {
      String times = send_time;
      Serial.println("time sent " + times);
      doc["action_time"] = times;
    }
    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    if (httpResponseCode == 200) {
      Serial.println("ack sent to server");
    }
    // Serial.println("HTTP Response code: " + String(httpResponseCode));

  } else if (!WiFi.isConnected()) {
    writeAttendanceSpiff(finger_id);
  }
}

void SendFingerprint(int finger_id, String send_time) {
  getAccessToken();
  if (WiFi.isConnected() && accessToken != "") {
    Serial.println("Sending the Fingerprint ID : ");
    Serial.println(finger_id);
    HTTPClient http;

    String url = URL + "/accounts/api/v1/user/fingerlog/";
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + accessToken);

    // JSON data
    StaticJsonDocument<200> doc;
    Serial.println("mac_id is " + device_token);
    doc["finger_id"] = finger_id;
    doc["mac_id"] = device_token;
    if (send_time != NULL) {
      String times = send_time;
      Serial.println("time sent " + times);
      doc["action_time"] = times;
    }
    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);
    String response = http.getString();
    Serial.println("HTTP Response code: " + String(httpResponseCode));
    Serial.println("Response: " + response);

    if (httpResponseCode == 201) {
      StaticJsonDocument<200> responseDoc;
      DeserializationError error = deserializeJson(responseDoc, response);
      if (error) {
        Serial.print(("deserializeJson() failed: "));
        Serial.println(error.f_str());
        http.end();  // End the HTTP connection
        return;
      }

      String firstname = responseDoc["first_name"];
      String lastname = responseDoc["last_name"];

      Serial.println("firstname: " + firstname);

      display.clearDisplay();
      display.setTextSize(1);       // Normal 2:2 pixel scale
      display.setTextColor(WHITE);  // Draw white text
      display.setCursor(15, 0);     // Start at top-left corner
      display.print(("Welcome"));
      display.setCursor(0, 10);
      display.print(firstname);
      display.setCursor(0, 20);
      display.print(lastname);
      display.setCursor(0, 30);
      display.print("id = ");
      display.print(finger_id);
      display.display();
      delay(2000);
    } else {
      writeAttendanceSpiff(finger_id);
    }

    http.end();  // End the HTTP connection
  } else if (!WiFi.isConnected()) {
    writeAttendanceSpiff(finger_id);
  }
}


void writeAttendanceSpiff(int finger_id) {
  myFile = SPIFFS.open(attendance_filename, "a");
  myFile.println(String(finger_id));
  myFile.println(time_format());
  myFile.close();

  Serial.println("atendance is written");
  display.println("atendance is written");
}

//***************disply ***********************
void displayScreen(String text) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(text);
  display.display();
}

//************get access token*************
void getAccessToken() {

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("getAccessToken func");
    HTTPClient http;
    http.begin(client, URL + "/accounts/api/v1/jwt/create/");
    http.addHeader("Content-Type", "application/json");

    // JSON data
    StaticJsonDocument<200> doc;
    doc["username"] = "IOT@gmail.com";
    doc["password"] = "12345678IOT";
    String requestBody;
    serializeJson(doc, requestBody);

    int httpResponseCode = http.POST(requestBody);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Token HTTP Response code: " + String(httpResponseCode));
      Serial.println("Token Response: " + response);

      // Parse JSON response
      StaticJsonDocument<200> responseDoc;
      DeserializationError error = deserializeJson(responseDoc, response);
      if (!error) {
        accessToken = responseDoc["access"].as<String>();
        refreshToken = responseDoc["refresh"].as<String>();
        Serial.println("Access Token: " + String(accessToken));
        Serial.println("Refresh Token: " + String(refreshToken));
      } else {
        Serial.println("Failed to parse JSON");
      }
    } else {
      Serial.println("Error on HTTP request in access token");
    }

    http.end();  // Free resources
  }
}

//************TEST send the fingerprint to the website*************
void checkEnroll(AsyncWebServerRequest *request) {
  Serial.println("enrolling ");
  // String employee_code = server.arg("employee_code");
  // String mac_id = server.arg("mac_id");
  // String first_name = server.arg("first_name");
  // String last_name = server.arg("last_name");
  String finger_id = request->arg("finger_id");
  // String pk = server.arg("pk");
  String user_pk = request->arg("user_pk");
  global_id = finger_id.toInt();
  user_pk_global = user_pk;
  request->send(200, "text/plain", "enrolled");

  check_enroll = true;
}

void ackEnroll(String user_pk) {
  getAccessToken();
  if (WiFi.isConnected() && accessToken != "") {
    HTTPClient http;  //Declare object of class HTTPClient
    //GET Data
    //GET methode
    String url = URL + "/accounts/api/v1/user/doorinsert/ack/";
    http.begin(client, url);  //initiate HTTP request   //Specify content-type header
    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", "Bearer " + accessToken);

    // JSON data
    StaticJsonDocument<200> doc;
    // doc["employee_code"] = employee_code;
    doc["mac_id"] = device_token;
    doc["door_id"] = global_id;
    // doc["first_name"] = first_name;
    // doc["last_name"] = last_name;
    doc["finger_data"] = "";
    // doc["pk"] = pk;
    doc["user_pk"] = user_pk;
    String requestBody;
    serializeJson(doc, requestBody);

    http.POST(requestBody);
    // Serial.println("HTTP Response code: " + String(httpResponseCode));

  } else if (!WiFi.isConnected()) {
    // writeAttendanceSpiff(finger_id);
  }
}

void checkDelete(AsyncWebServerRequest *request) {
  String id = request->arg("emp_id");
  // String user_pk = request->arg("user_pk");
  int id_int = id.toInt();
  Serial.println("id_received " + id_int);

  int p = deleteFingerprint(id_int);
  Serial.println("res " + p);

  request->send(200, "text/plain", "deleted");
}

//********************Get the Fingerprint ID******************
int getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println("No finger detected");
      return 0;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return -2;
    case FINGERPRINT_IMAGEFAIL:
      //Serial.println("Imaging error");
      return -2;
    default:
      //Serial.println("Unknown error");
      return -2;
  }
  // OK success!
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return -1;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return -2;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return -2;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return -2;
    default:
      //Serial.println("Unknown error");
      return -2;
  }
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return -2;
  } else if (p == FINGERPRINT_NOTFOUND) {
    //Serial.println("Did not find a match");
    return -1;
  } else {
    //Serial.println("Unknown error");
    return -2;
  }
  // found a match!
  Serial.print("Found ID #");
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of ");
  Serial.println(finger.confidence);
  dataSended = false;

  return finger.fingerID;
}

//******************Delete Finpgerprint ID*****************
uint8_t deleteFingerprint(int id) {
  uint8_t p = -1;
  Serial.print("id is ..." + id);
  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    //Serial.println("Deleted!");
    display.clearDisplay();
    display.setTextSize(2);       // Normal 2:2 pixel scale
    display.setTextColor(WHITE);  // Draw white text
    display.setCursor(0, 0);      // Start at top-left corner
    display.print(("Deleted!\n"));
    display.display();
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    display.clearDisplay();
    display.setTextSize(1);       // Normal 1:1 pixel scale
    display.setTextColor(WHITE);  // Draw white text
    display.setCursor(0, 0);      // Start at top-left corner
    display.print(("Communication error!\n"));
    display.display();
    Serial.println(id + " deleted");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    //Serial.println("Could not delete in that location");
    display.clearDisplay();
    display.setTextSize(1);       // Normal 1:1 pixel scale
    display.setTextColor(WHITE);  // Draw white text
    display.setCursor(0, 0);      // Start at top-left corner
    display.print(("Could not delete in that location!\n"));
    display.display();
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    //Serial.println("Error writing to flash");
    display.clearDisplay();
    display.setTextSize(1);       // Normal 1:1 pixel scale
    display.setTextColor(WHITE);  // Draw white text
    display.setCursor(0, 0);      // Start at top-left corner
    display.print(("Error writing to flash!\n"));
    display.display();
    return p;
  } else {
    //Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
    display.clearDisplay();
    display.setTextSize(2);       // Normal 2:2 pixel scale
    display.setTextColor(WHITE);  // Draw white text
    display.setCursor(0, 0);      // Start at top-left corner
    display.print(("Unknown error:\n"));
    display.display();
    return p;
  }
}


//******************Enroll a Finpgerprint ID*****************
uint8_t getFingerprintEnroll() {
  int p = -1;
  display.clearDisplay();
  display.drawBitmap(34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
  display.display();

  if (global_id <= 0) {
    Serial.println("global_id is 0");
    return -1;
  }

  unsigned long new_millis = millis();
  while (p != FINGERPRINT_OK) {
    if(millis()-new_millis > 10000) return p; 

    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        display.clearDisplay();
        display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
        display.display();
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.println(".");
        display.setTextSize(1);       // Normal 2:2 pixel scale
        display.setTextColor(WHITE);  // Draw white text
        display.setCursor(0, 0);      // Start at top-left corner
        display.print(("scanning"));
        display.display();
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        display.clearDisplay();
        display.drawBitmap(34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
        display.display();
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      display.clearDisplay();
      display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_IMAGEMESS:
      display.clearDisplay();
      display.drawBitmap(34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
      display.display();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }
  display.clearDisplay();
  display.setTextSize(2);       // Normal 2:2 pixel scale
  display.setTextColor(WHITE);  // Draw white text
  display.setCursor(0, 0);      // Start at top-left corner
  display.print(("Again"));
  display.display();
  //Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID ");
  Serial.println(global_id);
  p = -1;
  display.clearDisplay();
  display.drawBitmap(34, 0, FinPr_scan_bits, FinPr_scan_width, FinPr_scan_height, WHITE);
  display.display();
  new_millis = millis();
  while (p != FINGERPRINT_OK) {
    if(millis()-new_millis > 10000) return p; 

    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        //Serial.println("Image taken");
        display.clearDisplay();
        display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
        display.display();
        break;
      case FINGERPRINT_NOFINGER:
        //Serial.println(".");
        display.setTextSize(1);       // Normal 2:2 pixel scale
        display.setTextColor(WHITE);  // Draw white text
        display.setCursor(0, 0);      // Start at top-left corner
        display.print(("scanning"));
        display.display();
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      display.clearDisplay();
      display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
      display.display();
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      display.clearDisplay();
      display.drawBitmap(34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
      display.display();
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(global_id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    display.clearDisplay();
    display.drawBitmap(34, 0, FinPr_valid_bits, FinPr_valid_width, FinPr_valid_height, WHITE);
    display.display();
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    display.clearDisplay();
    display.drawBitmap(34, 0, FinPr_invalid_bits, FinPr_invalid_width, FinPr_invalid_height, WHITE);
    display.display();
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID ");
  Serial.println(global_id);
  p = finger.storeModel(global_id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    // display.clearDisplay();
    // display.print(("registered"));
    // display.display();
    delay(2000);
    // confirmAdding(id);
    return p;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }
}
//******************Check if there a Fingerprint ID to add******************
void writeMonitor(AsyncWebServerRequest *request) {
  myFile = SPIFFS.open(attendance_filename, "a");
  myFile.println(1);
  String timeNow = time_format();
  Serial.println(timeNow);
  displayScreen(timeNow);
  delay(2000);
  myFile.println(timeNow);

  myFile.close();

  request->send(200, "text/plain", "writeMonitor");
}

void readMonitor(AsyncWebServerRequest *request) {
  myFile = SPIFFS.open(attendance_filename, "r");
  if (!myFile) Serial.println("file open failed");
  int str[20];
  while (myFile.available()) {
    // Serial.write(myFile.read());
    // int i = 0;
    // str[i] = myFile.read();
    // i++;
    String inp = myFile.readString();

    int newlineIndex = inp.indexOf('\n');
    String firstPart = inp.substring(0, newlineIndex);
    String secondPart = inp.substring(newlineIndex + 1);

    // Print the separated strings
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("First Part: ");
    display.println(firstPart);
    display.print("Second Part: ");
    display.println(secondPart);
    display.display();
    delay(2000);
  }
  myFile.close();
  remove_file();

  request->send(200, "text/plain", "readMonitor");
}
