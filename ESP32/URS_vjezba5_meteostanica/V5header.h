#include <Dps3xx.h>
#include <DpsClass.h>

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


//Defines here
#define OLED_ROW_0 10
#define OLED_ROW_1 28
#define OLED_ROW_2 44
#define OLED_ROW_3 60

const unsigned char TCA9548A_DEF_ADDR = 0x70;  //<-- adresa i2c multipleksora
const unsigned char DPS310_IIC_ADDR = 0x77;
const unsigned char SHT35_IIC_ADDR = 0x45;
const unsigned char SCLPIN = 22;
//const unsigned char SDAPIN = 21;
//const unsigned char NONPIN = 255;
const unsigned char DPS310_IIC_CH = 0;
const unsigned char SHT35_IIC_CH = 1;
const unsigned char OLED_IIC_CH = 2;

