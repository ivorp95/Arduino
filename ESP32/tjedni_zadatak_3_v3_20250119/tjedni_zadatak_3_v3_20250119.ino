
#include <Adafruit_SHT31.h>

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_GenericDevice.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_DPS310.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>

#include <U8g2lib.h>

#include <Wire.h>


#define OLED_RESET 4
#define OLED_WIDTH 128
#define OLED_HEIGHT 64


const int touchPin = T0; //GPIO4      touch sensor spojiti na gpio4
bool dodir = false;
bool zadnjeStanjeDodira=false;
int vrDodira=0;
const int pragDodira=30;

float temperatura=0;
float vlagaZraka=0;
float tlakZraka=0;


enum Mode {TRENUTNO, STATISTIKA, RESET};
Mode displayMode = TRENUTNO;


float temperature, humidity, pressure;
float tempMin = 1000.0, tempMax = -1000.0;
float humMin = 100.0, humMax = -100.0;
float pressMin = 1000.0, pressMax = -1000.0;
float tempAvg = 0.0, humAvg = 0.0, pressAvg = 0.0;
unsigned long readingsCount = 0;


Adafruit_DPS310 dps;
Adafruit_SHT31 sht;  
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);


void setup(){

  Serial.begin(115200);
  Wire.begin();

  sht.begin(0x45);

  dps.begin_I2C(0x77);
  dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
  dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);

  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);

  //display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
}


void loop(){
  //delay(2000);

  TCA9548A(1);
  delay(500);
  temperatura=mjerenjeTemp();
  delay(500);
  vlagaZraka=mjerenjeVlag();
  delay(400);


  TCA9548A(0);
  delay(200);
  tlakZraka=mjerenjeTlak();
  delay(300);

  readSensors();

  vrDodira=touchRead(touchPin);
  dodir=vrDodira<pragDodira;

  if (dodir==true && zadnjeStanjeDodira==false){
  Serial.println(dodir);
  Serial.println(vrDodira);
  displayMode = static_cast<Mode>((displayMode + 1) % 3);

  delay(500);
  }

    switch (displayMode) {
    case TRENUTNO:
      u8g2ispisTemp(temperatura);
      delay(500);
      u8g2ispisVlag(vlagaZraka);
      delay(500);
      u8g2ispisTlak(tlakZraka);
      delay(500);

      break;
      
    case STATISTIKA:
      u8g2ProsjekTemp(tempMin,tempMax,tempAvg);
      delay(500);
      u8g2ProsjekTlak(pressMin,pressMax,pressAvg);
      delay(500);
      u8g2ProsjekVlag(humMin,humMax,humAvg);
      delay(500);
    
      break;
      

    case RESET:

    resetStatistics();

      break;

    //zadnjeStanjeDodira=dodir;

  }

  delay(100);
  


}






void TCA9548A(uint8_t bus){
  delay(100);
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  //Serial.println(bus);
  //delay(200);
}

void resetStatistics() {
  tempMin = 1000.0;
  tempMax = -1000.0;
  humMin = 100.0;
  humMax = -100.0;
  pressMin = 1000.0;
  pressMax = -1000.0;
  tempAvg = 0.0;
  humAvg = 0.0;
  pressAvg = 0.0;
  readingsCount = 0;

  TCA9548A(2);

  u8g2.clearBuffer();
  String msg2 = "Podaci resetirani";
  u8g2.drawStr(0, 10, msg2.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);
}

void u8g2ispisTemp(float temp){

  TCA9548A(2);

  u8g2.clearBuffer();
  String msg = "Temperatura: " + String(temp) + " C ";
  u8g2.drawStr(0, 10, msg.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
}


void u8g2ispisTlak(float tlak){

  TCA9548A(2);

  delay(100);
  u8g2.clearBuffer();
  String msg = "Tlak zraka: " + String(tlak) + " % ";
  u8g2.drawStr(0, 10, msg.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
}

void u8g2ispisVlag(float hum){

  TCA9548A(2);

  u8g2.clearBuffer();
  String msg = "Humidity: " + String(hum) + " C ";
  u8g2.drawStr(0, 10, msg.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
}

void u8g2ProsjekTemp(float tempMin,float tempMax,float tempAvg){

  TCA9548A(2);

  u8g2.clearBuffer();
  String msg = "TemMin: " + String(tempMin) + " C ";
  u8g2.drawStr(0, 10, msg.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);

  u8g2.clearBuffer();
  String msg1 = "TempMax: " + String(tempMax) + " C ";
  u8g2.drawStr(0, 10, msg1.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);

  u8g2.clearBuffer();
  String msg2 = "TemAvg: " + String(tempAvg) + " C ";
  u8g2.drawStr(0, 10, msg2.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);
}


void u8g2ProsjekTlak(float pressMin, float pressMax,float pressAvg){

  TCA9548A(2);

  u8g2.clearBuffer();
  String msg = "pressMin: " + String(pressMin) + " C ";
  u8g2.drawStr(0, 10, msg.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);

  u8g2.clearBuffer();
  String msg1 = "pressMax: " + String(pressMax) + " C ";
  u8g2.drawStr(0, 10, msg1.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);

  u8g2.clearBuffer();
  String msg2 = "pressAvg: " + String(pressAvg) + " C ";
  u8g2.drawStr(0, 10, msg2.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);
}



void u8g2ProsjekVlag(float humMin, float humMax, float humAvg){

  TCA9548A(2);

  u8g2.clearBuffer();
  String msg = "humMin: " + String(humMin) + " C ";
  u8g2.drawStr(0, 10, msg.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);

  u8g2.clearBuffer();
  String msg1 = "humMax: " + String(humMax) + " C ";
  u8g2.drawStr(0, 10, msg1.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);

  u8g2.clearBuffer();
  String msg2 = "humAvg: " + String(humAvg) + " C ";
  u8g2.drawStr(0, 10, msg2.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);
}



float mjerenjeVlag(){

  TCA9548A(1);
  float hum = sht.readHumidity();
  Serial.print("Vlaga zraka = "); 
  Serial.print(hum);
  Serial.println("%");

  delay(100);
  //ispisOLEDhum(hum);

  return hum;

}



float mjerenjeTemp(){

  TCA9548A(0);
  sensors_event_t temp_event, pressure_event;
  delay(1000);
  dps.getEvents( &temp_event, &pressure_event); 
  //Serial.print(F("Atmosferski pritisak = ")); 
  //Serial.print(pressure_event.pressure); 
  //Serial.println(" hPa");

  Serial.print(F("Temperatura = ")); 
  Serial.print(temp_event.temperature); 
  Serial.println(" *C");
  Serial.println();

  //u8g2ispisTemp(temp_event.temperature);
  //u8g2ispisTlak(pressure_event.pressure);

  //ispisOLEDtemp(temp_event.temperature);
  //ispisOLEDtlak(pressure_event.pressure);
  //delay(200);
  return temp_event.temperature;

}


float mjerenjeTlak(){

  TCA9548A(0);
  sensors_event_t temp_event, pressure_event;
  delay(1000);
  dps.getEvents( &temp_event, &pressure_event); 
  Serial.print(F("Atmosferski pritisak = ")); 
  Serial.print(pressure_event.pressure); 
  Serial.println(" hPa");

  //Serial.print(F("Temperatura = ")); 
  //Serial.print(temp_event.temperature); 
  //Serial.println(" *C");
  //Serial.println();

  //u8g2ispisTemp(temp_event.temperature);
  //u8g2ispisTlak(pressure_event.pressure);

  //ispisOLEDtemp(temp_event.temperature);
  //ispisOLEDtlak(pressure_event.pressure);
  //delay(200);
  return pressure_event.pressure;

}



void readSensors() {
  TCA9548A(0);
  temperature = mjerenjeTemp();
  pressure = mjerenjeTlak();

  TCA9548A(1);
  humidity = mjerenjeVlag();

  if (!isnan(temperature) && !isnan(humidity) && !isnan(pressure)) {
    // Ažuriraj minimum, maksimum i prosjek
    if (temperature < tempMin) tempMin = temperature;
    if (temperature > tempMax) tempMax = temperature;
    tempAvg = (tempAvg * readingsCount + temperature) / (readingsCount + 1);

    if (humidity < humMin) humMin = humidity;
    if (humidity > humMax) humMax = humidity;
    humAvg = (humAvg * readingsCount + humidity) / (readingsCount + 1);

    if (pressure < pressMin) pressMin = pressure;
    if (pressure > pressMax) pressMax = pressure;
    pressAvg = (pressAvg * readingsCount + pressure) / (readingsCount + 1);

    readingsCount++;
  }
}




/*
void ispisOLEDtemp(float temp){

  //delay(500);
  TCA9548A(2);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(2, 10);
  display.print("T= "); 
  display.print(temp); 
  display.print("*C");
  display.setTextSize(1);
  display.setCursor(2, 28);
  display.println("Veleri.Telematika");
  display.setCursor(2, 40);
  display.setTextSize(2);
  display.println("TEMPERATRA");
  display.display();
  delay(4000);

}



void ispisOLEDtlak(float tlak){
  //delay(500);
  TCA9548A(2);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(2, 10);
  display.print(tlak); 
  display.print("hPa"); 
  display.setTextSize(1);
  display.setCursor(2, 28);
  display.println("Veleri.Telematika");
  display.setCursor(2, 40);
  display.setTextSize(2);
  display.println("TLAK.ZRAKA");
  display.display();
  delay(4000);

}

void ispisOLEDhum(float hum){
  delay(500);
  //TCA9548A(2);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(2, 10);
  display.print("H= "); 
  display.print(hum);
  display.print("%"); 
  display.setTextSize(1);
  display.setCursor(2, 28);
  display.println("Veleri.Telematika");
  display.setCursor(0, 40);
  display.setTextSize(2);
  display.println("VLAGA.ZRAKA");
  display.display();
  delay(4000);

}
*/





