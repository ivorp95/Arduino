#include <mojHeader.h>

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

  mjerenjeTlakaTemp();

  //delay(100);
  mjerenjeVlag();

  //delay(2000);



}


void TCA9548A(uint8_t bus){
  delay(100);
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  //Serial.println(bus);
  //delay(200);
}


void mjerenjeVlag(){

  TCA9548A(1);
  float hum = sht.readHumidity();
  Serial.print("Vlaga zraka = "); 
  Serial.print(hum);
  Serial.println("%");

  delay(100);
  //ispisOLEDhum(hum);

  //return hum;

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


void mjerenjeTlakaTemp(){

  TCA9548A(0);
  sensors_event_t temp_event, pressure_event;
  delay(1000);
  dps.getEvents( &temp_event, &pressure_event); 
  Serial.print(F("Atmosferski pritisak = ")); 
  Serial.print(pressure_event.pressure); 
  Serial.println(" hPa");

  Serial.print(F("Temperatura = ")); 
  Serial.print(temp_event.temperature); 
  Serial.println(" *C");
  Serial.println();

  u8g2ispisTemp(temp_event.temperature);
  u8g2ispisTlak(pressure_event.pressure);

  //ispisOLEDtemp(temp_event.temperature);
  //ispisOLEDtlak(pressure_event.pressure);
  //delay(200);

}


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




void ispisOLEDhum(float hum){
  //delay(500);
  TCA9548A(2);
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

