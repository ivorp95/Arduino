
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>

#include <cmath> 
#include <cstdlib> 
#define err_t sht_err_t 
#include <Seeed_SHT35.h> 
#undef err_t
#include <U8g2lib.h> 
#include <Wire.h>
// Persistent storage related headers:
#include <Preferences.h>

#include "FS.h"
#include <LittleFS.h>
#define SPIFFS LittleFS 


#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncJson.h> 
#include <ESPAsyncWebServer.h> 
#include <HTTPClient.h> 
#include <time.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>

String ssid = "Ivors21";
String wifi_pass = "";

StaticJsonDocument<500> doc;
char jsonstring[] = "{\"key\":\"value\"}"; 
//deserializeJson(doc, jsonstring);



HTTPClient client; 

int GET();
int POST(uint8_t * payload, size_t size);
int POST(String payload);
int PUT(uint8_t * payload, size_t size);
int PUT(String payload);


String response;


String header(const char* name);
String header(size_t i);
String headerName(size_t i);
int headers();
// get request header value by name // get request header value by number // get request header name by number // get header count
bool hasHeader(const char* name);  // check if header exists


#define FORMAT_SPIFFS_IF_FAILED false
Preferences prefs;
AsyncWebServer server(80);

// Host name of the app server:
String hostname = "192.168.5.24";
unsigned int server_port = 3000;
unsigned int IDMeteoStation = 0;
const unsigned char SCLPIN = 22;
const unsigned char SHT35_IIC_ADDR = 0x45; 
SHT35 sensor(SCLPIN, SHT35_IIC_ADDR);
const int ALARM_PIN = 4;  // Alarm LED pin


const char* LOGFILE = "/sensor_log.csv";
// Log every half hour (1800 seconds):
int sample_count = 0;
const int LOG_EVERY  = 1800;


// Init display:
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);


// Sensor value storage:
enum Sensors {S_TEMP, S_HUMIDITY};  // Sensor IDs
enum AlarmLimit {A_LO, A_HI};
const int num_sensors = 2;
float sensor_value[2];

// Default alarm values:
float alarms[num_sensors][2] = {{18.0f, 23.0f}, {20.0f, 60.0f}};
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

// WWW root
const String www_header = "<!DOCTYPE html><html><head><title>Veleri-OI-meteo station</title><style>body {background-color: white;text-align: center;color: black;font- family: Arial, Helvetica, sans-serif;}</style></head><body><h1>Veleri-OI-meteo station</h1>";



