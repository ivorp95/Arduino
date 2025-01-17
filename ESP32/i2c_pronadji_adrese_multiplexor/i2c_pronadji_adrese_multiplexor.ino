#include <Wire.h>



void TCA9548A(uint8_t bus){
  Wire.beginTransmission(0x70);  // TCA9548A address is 0x70
  Wire.write(1 << bus);          // send byte to select bus
  Wire.endTransmission();
  //Serial.println(bus);
}




void setup() {
  Wire.begin();
  Serial.begin(115200);
}
void loop() {
int multiplex;


  Serial.println("Scan beginning.");
  byte errorcode;
  byte address;
  
  for(multiplex=0; multiplex<8;multiplex++){
    TCA9548A(multiplex);

    for(address = 1; address<128; address++) {

      Wire.beginTransmission(address);
      errorcode = Wire.endTransmission();

        if (errorcode == 0 && address!=112) {
          Serial.print("I2C device found at address 0x");
          Serial.println(address,HEX);
          Serial.print("ON BUS ");
          Serial.println(multiplex);
        }
      else if (errorcode==4) {
      Serial.print("Unknown error with device at address ");
      Serial.println(address);
    } }
  }
  Serial.println("Done scanning.\n");
  delay(10000);
}