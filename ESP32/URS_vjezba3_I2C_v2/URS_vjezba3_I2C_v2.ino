#include <mojHeader.h>


uint32_t start;
uint32_t stop;

float temperatura=0;
float vlagaZraka=0;
float tlakZraka=0;


Adafruit_SHT31 sht;
Adafruit_DPS310 dps;


void setup(){

  Serial.begin(115200);
  Wire.begin();

  sht.begin(0x45);
  delay(100);

  dps.begin_I2C(0x77);
  dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
  dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);
  delay(100);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  delay(100);

  scanI2Cbus(); //funkcija koja pronadje adrese, izvrsiti samo jednom
  delay(200);

  ispisOLED();

}

void loop(){

  delay(500);

  TCA9548A(1);
  delay(500);
  temperatura=mjerenjeTemp();
  delay(500);
  vlagaZraka=mjerenjeVlag();
  delay(400);


  TCA9548A(0);
  delay(200);
  tlakZraka=mjerenjeTlaka();
  delay(300);

  //delay(200);
  //TCA9548A(2);
  //ispisOLEDtlak(tlakZraka);
  //ispisOLEDtemp(temperatura);
  //ispisOLEDhum(vlagaZraka);



}









void TCA9548A(uint8_t bus){
  delay(100);
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  //Serial.println(bus);
  delay(200);
}



float mjerenjeTemp(){

  delay(200);
  
  float temp = sht.readTemperature();
  Serial.print("Temp *C = "); 
  Serial.print(temp); 
  Serial.print("	");
  delay(300);

  return temp;

}





float mjerenjeVlag(){

  delay(200);

  float hum = sht.readHumidity();

  Serial.print("Hum. % = "); 
  Serial.println(hum);

  delay(300);

  return hum;

}




float mjerenjeTlaka(){

  delay(300);
  TCA9548A(0);
  sensors_event_t temp_event, pressure_event;
  delay(100);
  dps.getEvents( &temp_event, &pressure_event); 
  Serial.print(F("Pressure = ")); 
  Serial.print(pressure_event.pressure); 
  Serial.println(" hPa");
  Serial.println();

  delay(200);

  return pressure_event.pressure;

}




void ispisOLED(){

  delay(800);
  TCA9548A(2);

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(40, 10);
  display.print("OLED");
  display.setTextSize(1);
  display.setCursor(23, 28);
  display.println("www.joy-it.net");
  display.setCursor(8, 40);
  display.setTextSize(2);
  display.println("SBC-OLED01");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.invertDisplay(true);
  delay(2000);
  display.invertDisplay(false);
  delay(1000);

}


void ispisOLEDtemp(float temp){

  delay(500);
  //TCA9548A(2);

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




void ispisOLEDtlak(float tlak){
  delay(500);
  //TCA9548A(2);
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




void scanI2Cbus(){

  uint8_t error, i2cAddress, devCount, unCount;

  Serial.println("Scanning...");
  devCount = 0;
  unCount = 0;
  for(i2cAddress = 1; i2cAddress < 127; i2cAddress++ ){
    Wire.beginTransmission(i2cAddress);
    error = Wire.endTransmission();

    if (error == 0){
      Serial.print("I2C device found at 0x");
      if (i2cAddress<16) Serial.print("0");
      Serial.println(i2cAddress,HEX);
      devCount++;
    }
    else if (error==4){
      Serial.print("Unknow error at 0x");
      if (i2cAddress<16) Serial.print("0");
      Serial.println(i2cAddress,HEX);
      unCount++;
    }    
  }
  if (devCount + unCount == 0)
    Serial.println("No I2C devices found\n");
  else {
    Serial.print(devCount);
    Serial.print(" device(s) found");
    if (unCount > 0) {
      Serial.print(", and unknown error in ");
      Serial.print(unCount);
      Serial.print(" address");
    }
    Serial.println();
  }
  Serial.println();
  delay(1000); 
}