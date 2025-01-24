#include "URSheader.h"




void setup() {
  Serial.begin(115200);

  Alarms alarms;
  prefs.begin("Veleri-OI-meteo", false);
  bool hasPrefs = prefs.getBool("valid", false);

  if (hasPrefs){
    Serial.println("Saved settings found. ");
    float tmp;
    // Read settings into a temporary variable and store if no error.
    // In case of missing key-value pair or other error, getFloat returns NaN
    tmp = prefs.getFloat("humidity_low");
    if (!isnan(tmp)) alarms.humidity_low = tmp;
    tmp = prefs.getFloat("humidity_high");
    if (!isnan(tmp)) alarms.humidity_high = tmp;
    tmp = prefs.getFloat("temperature_low");
    if (!isnan(tmp)) alarms.temperature_low = tmp;
    tmp = prefs.getFloat("temperature_high");
    if (!isnan(tmp)) alarms.temperature_high = tmp;
  }
  else{
    Serial.println("Saved settings not found, using default values."); 
    prefs.putFloat("humidity_low", alarms.humidity_low); 
    prefs.putFloat("humidity_high", alarms.humidity_high); 
    prefs.putFloat("temperature_low", alarms.temperature_low); 
    prefs.putFloat("temperature_high", alarms.temperature_high); 
    prefs.putBool("valid", true);
  }
  Serial.println("Configured alarms: ");
  Serial.print("Min. temp.:");
  Serial.println(alarms.temperature_low);
  Serial.print("Max. temp.:");
  Serial.println(alarms.temperature_high);
  Serial.print("Min. humidity.:");
  Serial.println(alarms.humidity_low);
  Serial.print("Max. humidity.:");
  Serial.println(alarms.humidity_high);

  select_channel_i2c(OLED_IIC_CH);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);//8px font

  select_channel_i2c(DPS310_IIC_CH);
  Dps3xxPressureSensor.begin(Wire);

  select_channel_i2c(SHT35_IIC_CH);
  if (sensor.init())
    Serial.println("SHT35 init failed!");

  delay(1000);
  Serial.println("Init complete!");
}






void loop() {

  uint8_t oversampling = 7;
  int16_t ret;
  String msg = ""; //priprema poruke koja ce se poslati na serial i u buffer za OLED (SSD1306
  Serial.println();
  u8g2.clearBuffer(); // pocisti buffer koji sadrzi ono sto ce se prikazati na OLED. Buffer je u RAM MCU-a.




  vrDodira=touchRead(touchPin);
  dodir=vrDodira<pragDodira;
  if (dodir==true && zadnjeStanjeDodira==false){
    displayMode = static_cast<Mode>((displayMode + 1) % 4);


    switch (displayMode){

      case TRENUTNO:
      do{
        u8g2.clearBuffer(); 
        mjerenjeDPS();
        mjerenjeSHT();
        select_channel_i2c(OLED_IIC_CH);//odaberi na mux-u kanal
        u8g2.sendBuffer();//posalji na SSD1306
        delay(1000);
      }while((vrDodira=touchRead(touchPin))>pragDodira);
        break;

      case STATISTIKA:
        do{
        u8g2.clearBuffer(); 
        prosjeci();
        select_channel_i2c(OLED_IIC_CH);//odaberi na mux-u kanal
        u8g2.sendBuffer();//posalji na SSD1306
        delay(1000);
        }while((vrDodira=touchRead(touchPin))>pragDodira);
        break;

      case IZBORNIK:

        break;

      case RESET:
        resetStatistics();
        break;

    }
  }

}



void select_channel_i2c(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_DEF_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}


void mjerenjeDPS(){

  select_channel_i2c(DPS310_IIC_CH);
  /*
   * lets the Dps3xx perform a Single temperature measurement with the last (or standard) configuration
   * The result will be written to the parameter temperature
   * ret = Dps3xxPressureSensor.measureTempOnce(temperature);
   * the commented line below does exactly the same as the one above, but you can also config the precision
   * oversampling can be a value from 0 to 7
   * the Dps 3xx will perform 2^oversampling internal temperature measurements and combine them to one result with higher precision
   * measurements with higher precision take more time, consult datasheet for more information
   */
  ret = Dps3xxPressureSensor.measureTempOnce(temp, oversampling); //mjeri temperaturu, oversampling - uzmi vise uzoraka (2^n, n=0..7)

  if (ret != 0) {
    msg = "Temp@DPS FAIL! ret = " + String(ret);
  } else {
    msg = "Temp@DPS: " + String(temp) + " *C"; //napravi poruku
  }
  Serial.println(msg);
  u8g2.drawStr(0, OLED_ROW_0, msg.c_str()); // stavi poruku u buffer, ovo je jos u RAM MCU-a


  ret = Dps3xxPressureSensor.measurePressureOnce(press, oversampling);
  if (ret != 0) {
    msg = "Pressure read FAIL! ret = " + String(ret);
  } else {
    msg = "Pressure: " + String(press / 100) + " hPa";
  }
  Serial.println(msg);
  u8g2.drawStr(0, OLED_ROW_1, msg.c_str());


}


void mjerenjeSHT(){


  select_channel_i2c(SHT35_IIC_CH);
  if (NO_ERROR == sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temp, &hum)) {
    msg = "Temp@SHT: " + String(temp) + " *C ";
    Serial.println(msg);
    u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
    msg = "Humidity: " + String(hum) + " % ";
    u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
    Serial.println(msg);
  } else {
    msg = "Temp@SHT35 FAIL!";
    Serial.println(msg);
    u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
    msg = "Humid@SHT35 FAIL!";
    Serial.println(msg);
    u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
  }
}


void prosjeci() {
  

  select_channel_i2c(SHT35_IIC_CH);
  sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temp, &hum);

  select_channel_i2c(DPS310_IIC_CH);
  ret = Dps3xxPressureSensor.measurePressureOnce(press, oversampling);

  String msg;


  if (!isnan(temp) && !isnan(hum) && !isnan(press)) {
    if (temp < tempMin) tempMin = temp;
    if (temp > tempMax) tempMax = temp;


    tempAvg = (tempAvg * readingsCount + temp) / (readingsCount + 1);

    if (hum < humMin) humMin = hum;
    if (hum > humMax) humMax = hum;


    humAvg = (humAvg * readingsCount + hum) / (readingsCount + 1);

    if (press < pressMin) pressMin = press;
    if (press > pressMax) pressMax = press;

    
    pressAvg = (pressAvg * readingsCount + press) / (readingsCount + 1);

    readingsCount++;

    Serial.print("Avg. Temp.:");
    Serial.println(tempAvg);
    Serial.print("Avg. humidity.:");
    Serial.println(humAvg);
    Serial.print("Avg. pressure.:");
    Serial.println(pressAvg/100);


    msg = "TempAVG: " + String(tempAvg) + " *C ";
    u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
    msg = "HumAVG: " + String(humAvg) + " % ";
    u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
    msg = "PressAVG: " + String(pressAvg/100) + " hPa ";
    u8g2.drawStr(0, OLED_ROW_2, msg.c_str());

  }
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

  select_channel_i2c(OLED_IIC_CH);

  u8g2.clearBuffer();
  String msg2 = "Podaci resetirani";
  u8g2.drawStr(0, 10, msg2.c_str());
  //Serial.println(msg);
  u8g2.sendBuffer();
  delay(500);
}



