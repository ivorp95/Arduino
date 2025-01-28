
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
#define FORMAT_SPIFFS_IF_FAILED false

#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncJson.h> 
#include <ESPAsyncWebServer.h> 
#include <HTTPClient.h> 
#include <time.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>



