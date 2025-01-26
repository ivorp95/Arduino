#include <Preferences.h>
#include "Seeed_SHT35.h"
#include <Dps3xx.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <cmath>

#include "FS.h"
#include <LittleFS.h>
#define SPIFFS LittleFS 

//Defines here
#define OLED_ROW_0 15
#define OLED_ROW_1 31
#define OLED_ROW_2 47
#define OLED_ROW_3 63

//Constants here
const unsigned char TCA9548A_DEF_ADDR = 0x70;  //<-- adresa i2c multipleksora
//const unsigned char DPS310_IIC_ADDR = 0x77;
const unsigned char SHT35_IIC_ADDR = 0x45;
const unsigned char SCLPIN = 22;
//const unsigned char SDAPIN = 21;
//const unsigned char NONPIN = 255;
const unsigned char DPS310_IIC_CH = 0;
const unsigned char SHT35_IIC_CH = 1;
const unsigned char OLED_IIC_CH = 2;

struct Alarms{
  float humidity_low = 10;
  float humidity_high = 90;
  float temperature_low = 2;
  float temperature_high = 33;
};

float temp;
float press;
float hum;
uint8_t oversampling = 7;
int16_t ret;
String msg = ""; //priprema poruke koja ce se poslati na serial i u buffer za OLED (SSD1306)
Preferences prefs;

enum Mode {NULA, TRENUTNO, STATISTIKA, IZBORNIK};
Mode displayMode = NULA;

char unos;
const int touchPin = T0; //GPIO4      touch sensor spojiti na gpio4
bool dodir = false;
bool zadnjeStanjeDodira=false;
int vrDodira=0;
const int pragDodira=30;

float tempMin = 1000.0, tempMax = -1000.0;
float humMin = 100.0, humMax = -100.0;
float pressMin = 1000.0, pressMax = -1000.0;
float tempAvg = 0.0, humAvg = 0.0, pressAvg = 0.0;
unsigned long readingsCount = 0;



//Gloabals here

// Objectes here
Dps3xx Dps3xxPressureSensor = Dps3xx();
SHT35 sensor(SCLPIN, SHT35_IIC_ADDR);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, NONPIN, SCLPIN, SDAPIN);  //opcionalni parametri


