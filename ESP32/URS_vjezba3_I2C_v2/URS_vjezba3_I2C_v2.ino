#include <mojHeader.h>




uint32_t start;
uint32_t stop;


Adafruit_SHT31 sht;

Adafruit_DPS310 dps;


void setup(){

  Serial.begin(115200);
  Wire.begin();

  sht.begin(0x45);

  dps.begin_I2C(0x77);
  dps.configurePressure(DPS310_64HZ, DPS310_64SAMPLES);
  dps.configureTemperature(DPS310_64HZ, DPS310_64SAMPLES);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  scanI2Cbus(); //funkcija koja pronadje adrese, izvrsiti samo jednom

  ispisOLED();

}

void loop(){


  delay(500);
  TCA9548A(1);
  delay(500);
  mjerenjeTempVlag();

  delay(500);
  TCA9548A(0);
  delay(500);
  mjerenjeTlaka();



}









void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  //Serial.print(bus);
  delay(200);
}



void mjerenjeTempVlag(){
  
  float temp = sht.readTemperature();
  float hum = sht.readHumidity();

  Serial.print("Temp *C = "); 
  Serial.print(temp); 
  Serial.print("	");
  Serial.print("Hum. % = "); 
  Serial.println(hum);

  delay(1000);
  TCA9548A(2);
  delay(1000);
  ispisOLEDtemp(temp);
  delay(2000);
  ispisOLEDhum(hum);
  delay(2000);


}




void mjerenjeTlaka(){

  sensors_event_t temp_event, pressure_event;
  delay(1000);
  dps.getEvents(&temp_event, &pressure_event); 
  //Serial.print(F("Temperature = ")); 
  //Serial.print(temp_event.temperature); 
  //Serial.println(" *C");
  Serial.print(F("Pressure = ")); 
  Serial.print(pressure_event.pressure); 
  Serial.println(" hPa");
  Serial.println();

}




void ispisOLED(){

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
  delay(8000);
  display.clearDisplay();
  display.invertDisplay(true);
  delay(8000);
  display.invertDisplay(false);
  delay(1000);

}


void ispisOLEDtemp(float temp){

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(2, 10);
  display.print("Temp *C "); 
  display.print(temp); 
  display.setTextSize(1);
  display.setCursor(2, 28);
  display.println("Veleri.Telematika");
  display.setCursor(2, 40);
  display.setTextSize(2);
  display.println("TEMPERATRA");
  display.display();
  delay(2000);

}




void ispisOLEDhum(float hum){

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(2);
  display.setCursor(2, 10);
  display.print("Hum % "); 
  display.print(hum); 
  display.setTextSize(1);
  display.setCursor(2, 28);
  display.println("Veleri.Telematika");
  display.setCursor(2, 40);
  display.setTextSize(2);
  display.println("VLAGA.ZRAKA");
  display.display();
  delay(2000);

}




void scanI2Cbus(){

  uint8_t error, i2cAddress, devCount, unCount;

  Serial.println("Scanning...");
  devCount = 0;
  unCount = 0;
  for(i2cAddress = 1; i2cAddress < 127; i2cAddress++ )
  {
    Wire.beginTransmission(i2cAddress);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at 0x");
      if (i2cAddress<16) Serial.print("0");
      Serial.println(i2cAddress,HEX);
      devCount++;
    }
    else if (error==4)
    {
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
