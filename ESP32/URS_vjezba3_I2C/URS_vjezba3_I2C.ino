#include <mojHeader.h>


SHT35 sensor(SCLPIN);


void setup(){

  SERIAL.begin(115200);
  while(!SERIAL){};

  //WIRE.begin();
  TCA.begin(WIRE);
  TCA.openChannel(TCA_CHANNEL_0);   //TCA.closeChannel(TCA_CHANNEL_0);
  TCA.openChannel(TCA_CHANNEL_1); //TCA.closeChannel(TCA_CHANNEL_1);
  TCA.openChannel(TCA_CHANNEL_2); //TCA.closeChannel(TCA_CHANNEL_2);
  TCA.openChannel(TCA_CHANNEL_3); //TCA.closeChannel(TCA_CHANNEL_3);
  TCA.openChannel(TCA_CHANNEL_4); //TCA.closeChannel(TCA_CHANNEL_4);
  TCA.openChannel(TCA_CHANNEL_5); //TCA.closeChannel(TCA_CHANNEL_5);
  TCA.openChannel(TCA_CHANNEL_6); //TCA.closeChannel(TCA_CHANNEL_6);
  TCA.openChannel(TCA_CHANNEL_7); //TCA.closeChannel(TCA_CHANNEL_7); 

  if(sensor.init()){
    SERIAL.println("sensor init failed!!!");
  }


  scanI2Cbus(); //funkcija koja pronadje adrese, izvrsiti samo jednom

}

void loop(){

mjerenjeTempVlag();



}











void mjerenjeTempVlag(){

    u16 value=0;
    u8 data[6]={0};
    float temp,hum;
    if(NO_ERROR!=sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH,&temp,&hum))
    {
      SERIAL.println("read temp failed!!");
      SERIAL.println("   ");
      SERIAL.println("   ");
      SERIAL.println("   ");
    }
    else
    {
      SERIAL.println("result======>");
      SERIAL.print("temperature =");
      SERIAL.println(temp);

      SERIAL.print("humidity =");
      SERIAL.println(hum);

      SERIAL.println("   ");
      SERIAL.println("   ");
      SERIAL.println("   ");
    }
    delay(1000);

}









void mjerenjeTlaka(){

}



void scanI2Cbus(){

  uint8_t error, i2cAddress, devCount, unCount;

  SERIAL.println("Scanning...");
  devCount = 0;
  unCount = 0;
  for(i2cAddress = 1; i2cAddress < 127; i2cAddress++ )
  {
    WIRE.beginTransmission(i2cAddress);
    error = WIRE.endTransmission();

    if (error == 0)
    {
      SERIAL.print("I2C device found at 0x");
      if (i2cAddress<16) SERIAL.print("0");
      SERIAL.println(i2cAddress,HEX);
      devCount++;
    }
    else if (error==4)
    {
      SERIAL.print("Unknow error at 0x");
      if (i2cAddress<16) SERIAL.print("0");
      SERIAL.println(i2cAddress,HEX);
      unCount++;
    }    
  }

  if (devCount + unCount == 0)
    SERIAL.println("No I2C devices found\n");
  else {
    SERIAL.print(devCount);
    SERIAL.print(" device(s) found");
    if (unCount > 0) {
      SERIAL.print(", and unknown error in ");
      SERIAL.print(unCount);
      SERIAL.print(" address");
    }
    SERIAL.println();
  }
  SERIAL.println();
  delay(1000); 
}
