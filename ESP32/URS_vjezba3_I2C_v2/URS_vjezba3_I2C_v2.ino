
#include <mojHeader.h>



void setup(){

  Serial.begin(115200);
  Wire.begin();
  sht.begin(0x45);

  Wire.setClock(100000);
  uint16_t stat = sht.readStatus();
  Serial.print(stat, HEX);
  

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  scanI2Cbus(); //funkcija koja pronadje adrese, izvrsiti samo jednom

}

void loop(){

mjerenjeTempVlag();



}









void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  Serial.print(bus);
}



void mjerenjeTempVlag(){
  sht.read();

  Serial.print("Temperature:");
  Serial.println(sht.getTemperature(), 1);
  Serial.print("\t");
  Serial.print("Humidity:");
  Serial.println(sht.getHumidity(), 1);
  delay(50);


}








void mjerenjeTlaka(){

}


void openChannel(){

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
