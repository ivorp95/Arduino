#include <Preferences.h>
#include "Seeed_SHT35.h"
#include <Dps3xx.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <cmath>

#include "FS.h"
#include <LittleFS.h>
#define SPIFFS LittleFS 
#define FORMAT_SPIFFS_IF_FAILED false

#define OLED_ROW_0 15
#define OLED_ROW_1 31
#define OLED_ROW_2 47
#define OLED_ROW_3 63

const unsigned char TCA9548A_DEF_ADDR = 0x70; 
//const unsigned char DPS310_IIC_ADDR = 0x77;
const unsigned char SHT35_IIC_ADDR = 0x45;
const unsigned char SCLPIN = 22;
//const unsigned char SDAPIN = 21;
//const unsigned char NONPIN = 255;
const unsigned char DPS310_IIC_CH = 0;
const unsigned char SHT35_IIC_CH = 1;
const unsigned char OLED_IIC_CH = 2;

char* LOGFILE = "/sensor_log.csv";
// Log every half hour (1800 seconds):
int sample_count = 0;
const int LOG_EVERY  = 600;

enum Sensors {S_TEMP, S_HUMIDITY};  // Sensor IDs
enum AlarmLimit {A_LO, A_HI};
const int num_sensors = 2;
float sensor_value[2];

float alarms[num_sensors][2] = {{18.0f, 27.0f}, {20.0f, 70.0f}};
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;

unsigned long startMillis; 
const unsigned long period = 1000; 


float temp;
float press;
uint8_t oversampling = 7;
int16_t ret;
String msg = ""; //priprema poruke koja ce se poslati na serial i u buffer za OLED (SSD1306)
Preferences prefs;

enum Menu {NULA, TRENUTNO, STATISTIKA, IZBORNIK};
Menu menu_items = NULA;
const int NUM_ITEMS = 4; 


float tempMin = 1000.0, tempMax = -1000.0;
float humMin = 100.0, humMax = -100.0;
float pressMin = 1000.0, pressMax = -1000.0;
float tempAvg = 0.0, humAvg = 0.0, pressAvg = 0.0;
unsigned long readingsCount = 0;

Dps3xx Dps3xxPressureSensor = Dps3xx();
SHT35 sensor(SCLPIN, SHT35_IIC_ADDR);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, NONPIN, SCLPIN, SDAPIN);  //opcionalni parametri



#define BUTTON_UP_PIN 32 // pin for UP button 
#define BUTTON_SELECT_PIN 33 // pin for SELECT button
#define BUTTON_DOWN_PIN 25 // pin for DOWN button

int button_up_clicked = 0; // only perform action when button is clicked, and wait until another press
int button_select_clicked = 0; // same as above
int button_down_clicked = 0; // same as above

int item_selected = 0; // which item in the menu is selected

int item_sel_previous; // previous item - used in the menu screen to draw the item before the selected one
int item_sel_next; // next item - used in the menu screen to draw next item after the selected one

int current_screen = menu_items;   



void setup() {

  Serial.begin(115200);

  initPrefs();

  prefs.begin("Veleri-OI-meteo", false);


  select_channel_i2c(OLED_IIC_CH);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);//8px font
  msg="Vjezba 4";
  u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
  u8g2.sendBuffer();

  select_channel_i2c(DPS310_IIC_CH);
  Dps3xxPressureSensor.begin(Wire);

  select_channel_i2c(SHT35_IIC_CH);
  if (sensor.init())
    Serial.println("SHT35 init failed!");

  delay(1000);
  Serial.println("Init complete!");

    if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  //displayFile(SPIFFS, u8g2, LOGFILE);
  delay(1500);
  previousMillis = millis();

  pinMode(BUTTON_UP_PIN, INPUT_PULLUP); // up button
  pinMode(BUTTON_SELECT_PIN, INPUT_PULLUP); // select button
  pinMode(BUTTON_DOWN_PIN, INPUT_PULLUP); // down button
}






void loop() {

if (current_screen == 0) { 


    select_channel_i2c(OLED_IIC_CH);
    u8g2.clearBuffer();
    //u8g2.clearDisplay();
    msg = "Izbornik: ";
    u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
    msg = "Lijevo-TRENUTNO";
    u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
    msg = "Desno-PREOSJEK";
    u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
    msg = "Sredina-POSTAVKE";
    u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
    u8g2.sendBuffer();


      if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) {
        item_selected = item_selected - 1; // select previous item
        do{
        currentMillis = millis();
        if (currentMillis - previousMillis >= 1000) {
            handleSensors();
            handleAlarms();
            previousMillis = currentMillis; // Remember time of this reading.
            }
        } while(digitalRead(BUTTON_UP_PIN) == HIGH);
        button_up_clicked = 1; 
        if (item_selected < 0) { 
          item_selected = NUM_ITEMS-1;
        }
      }
      if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_down_clicked == 0)) {
        item_selected = item_selected + 1;
            do{
              currentMillis = millis();
              if (currentMillis - previousMillis >= 1000) {
              prosjeci();
              handleAlarms();
              previousMillis = currentMillis; // Remember time of this reading.
            }
            }while((digitalRead(BUTTON_DOWN_PIN) == HIGH));

            button_down_clicked = 1; 
            if (item_selected >= NUM_ITEMS) { 
            item_selected = 0;
            }
      } 

      if ((digitalRead(BUTTON_UP_PIN) == HIGH) && (button_up_clicked == 1)) { // unclick 
        button_up_clicked = 0;
      }
      if ((digitalRead(BUTTON_DOWN_PIN) == HIGH) && (button_down_clicked == 1)) { // unclick
        button_down_clicked = 0;
      }



    if ((digitalRead(BUTTON_SELECT_PIN) == LOW) && (button_select_clicked == 0)) { 
     button_select_clicked = 1; 
      select_channel_i2c(OLED_IIC_CH);
      u8g2.clearBuffer();
      //u8g2.clearDisplay();
      msg = "POSTAVKE Alarma++:";
      u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
      msg = "Lijevo-Alarm hum++";
      u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
      msg = "Desno-Alarm temp++";
      u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
      msg = "Sredina 2sec za izlaz";
      u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
      u8g2.sendBuffer();
      currentMillis=millis();
        do{ 

          if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) {
          button_select_clicked = 0; 
            do{
              u8g2.clearBuffer();
              //u8g2.clearDisplay();
              msg = "POSTAVKE,Alarm hum++:";
              u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
              msg = "Lijevo-Hum HI ++:"+String(alarms[S_HUMIDITY][A_HI]);
              u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
              msg = "Desno-Hum LO++:"+String(alarms[S_HUMIDITY][A_LO]);
              u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
              msg = "Sred+LijevoIliDesno->izlaz";
              u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
              u8g2.sendBuffer();

              if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) {
              alarms[S_HUMIDITY][A_HI]++;
              button_up_clicked = 0;
              }
              if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_down_clicked == 0)) {
              alarms[S_HUMIDITY][A_LO]++;
              button_down_clicked = 0;
              }
            }while((digitalRead(BUTTON_SELECT_PIN) == HIGH));
        }


        if ((digitalRead(BUTTON_UP_PIN) == HIGH) && (button_up_clicked == 1)) { // unclick 
        button_up_clicked = 0;
        }
        if ((digitalRead(BUTTON_DOWN_PIN) == HIGH) && (button_down_clicked == 1)) { // unclick
        button_down_clicked = 0;
        }


              
        currentMillis = millis();
      }while((digitalRead(BUTTON_DOWN_PIN) == HIGH)&& currentMillis>2000);




//////////////////////////////////////////////////////////////
    do{ 

          if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_up_clicked == 0)) {
          button_select_clicked = 0; 
            do{
              u8g2.clearBuffer();
              //u8g2.clearDisplay();
              msg = "POSTAVKE,Alarm temp++:";
              u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
              msg = "Lijevo-Temp HI ++:"+String(alarms[S_TEMP][A_HI]);
              u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
              msg = "Desno-Temp LO++:"+String(alarms[S_TEMP][A_LO]);
              u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
              msg = "Sred+LijevoIliDesno->izlaz";
              u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
              u8g2.sendBuffer();

              if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) {
              alarms[S_TEMP][A_HI]++;
              button_up_clicked = 0;
              }
              if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_down_clicked == 0)) {
              alarms[S_TEMP][A_LO]++;
              button_down_clicked = 0;
              }
            }while((digitalRead(BUTTON_SELECT_PIN) == HIGH));
        }


        if ((digitalRead(BUTTON_UP_PIN) == HIGH) && (button_up_clicked == 1)) { // unclick 
        button_up_clicked = 0;
        }
        if ((digitalRead(BUTTON_DOWN_PIN) == HIGH) && (button_down_clicked == 1)) { // unclick
        button_down_clicked = 0;
        }

      }while((digitalRead(BUTTON_DOWN_PIN) == HIGH)&& currentMillis>2000);
    }

    if ((digitalRead(BUTTON_SELECT_PIN) == HIGH) && (button_select_clicked == 1)) { 
      button_select_clicked = 0;
    }
///////////////////////////////////////////////////////////////

    previousMillis=millis();

   if ((digitalRead(BUTTON_SELECT_PIN) == LOW)) { 
      button_select_clicked=1;
      currentMillis=millis();


      if((digitalRead(BUTTON_SELECT_PIN) == LOW)&&(currentMillis-previousMillis>=3000)){

      button_select_clicked = 0; 
      select_channel_i2c(OLED_IIC_CH);
      u8g2.clearBuffer();
      //u8g2.clearDisplay();
      msg = "POSTAVKE Alarma--:";
      u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
      msg = "Lijevo-Alarm hum--";
      u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
      msg = "Desno-Alarm temp--";
      u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
      msg = "Sred+LijevoIliDesno->izlaz";
      u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
      u8g2.sendBuffer();
        do{ 

          if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) {
          button_select_clicked = 0; 
            do{
              u8g2.clearBuffer();
              //u8g2.clearDisplay();
              msg = "POSTAVKE,Alarm hum--:";
              u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
              msg = "Lijevo-Hum HI --:"+String(alarms[S_HUMIDITY][A_HI]);
              u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
              msg = "Desno-Hum LO--:"+String(alarms[S_HUMIDITY][A_LO]);
              u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
              msg = "Sred+LijevoIliDesno->izlaz";
              u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
              u8g2.sendBuffer();

              if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) {
              alarms[S_HUMIDITY][A_HI]--;
              button_up_clicked = 0;
              }
              if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_down_clicked == 0)) {
              alarms[S_HUMIDITY][A_LO]--;
              button_down_clicked = 0;
              }
            }while((digitalRead(BUTTON_SELECT_PIN) == HIGH));
        }


        if ((digitalRead(BUTTON_UP_PIN) == HIGH) && (button_up_clicked == 1)) { // unclick 
        button_up_clicked = 0;
        }
        if ((digitalRead(BUTTON_DOWN_PIN) == HIGH) && (button_down_clicked == 1)) { // unclick
        button_down_clicked = 0;
        }


              
        currentMillis = millis();
      }while((digitalRead(BUTTON_DOWN_PIN) == HIGH)&& currentMillis>2000);




    do{ 

          if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_up_clicked == 0)) {
          button_select_clicked = 0; 
            do{
              u8g2.clearBuffer();
              //u8g2.clearDisplay();
              msg = "POSTAVKE,Alarm temp--:";
              u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
              msg = "Lijevo-Temp HI --:"+String(alarms[S_TEMP][A_HI]);
              u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
              msg = "Desno-Temp LO--:"+String(alarms[S_TEMP][A_LO]);
              u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
              msg = "Sred+LijevoIliDesno->izlaz";
              u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
              u8g2.sendBuffer();

              if ((digitalRead(BUTTON_UP_PIN) == LOW) && (button_up_clicked == 0)) {
              alarms[S_TEMP][A_HI]--;
              button_up_clicked = 0;
              }
              if ((digitalRead(BUTTON_DOWN_PIN) == LOW) && (button_down_clicked == 0)) {
              alarms[S_TEMP][A_LO]--;
              button_down_clicked = 0;
              }
            }while((digitalRead(BUTTON_SELECT_PIN) == HIGH));
        }


        if ((digitalRead(BUTTON_UP_PIN) == HIGH) && (button_up_clicked == 1)) { // unclick 
        button_up_clicked = 0;
        }
        if ((digitalRead(BUTTON_DOWN_PIN) == HIGH) && (button_down_clicked == 1)) { // unclick
        button_down_clicked = 0;
        }
        
      }while((digitalRead(BUTTON_DOWN_PIN) == HIGH)&& currentMillis>2000);
    }

    if ((digitalRead(BUTTON_SELECT_PIN) == HIGH) && (button_select_clicked == 1)) { 
      button_select_clicked = 0;
    }
    ////////////////


  }

    

}
  

}










void select_channel_i2c(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_DEF_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void handleSensors() {
  int16_t ret;
  String msg;    
  select_channel_i2c(DPS310_IIC_CH);

  ret = Dps3xxPressureSensor.measureTempOnce(temp, oversampling); //mjeri temperaturu, oversampling - uzmi vise uzoraka (2^n, n=0..7)
  if (ret != 0) {
    msg = "Temp@DPS FAIL! ret = " + String(ret);
  } else {
    msg = "Temp@DPS: " + String(temp) + " *C"; //napravi poruku
  }
  Serial.println(msg);
  select_channel_i2c(OLED_IIC_CH);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(0, OLED_ROW_2, msg.c_str()); // stavi poruku u buffer, ovo je jos u RAM MCU-a

  select_channel_i2c(DPS310_IIC_CH);
  ret = Dps3xxPressureSensor.measurePressureOnce(press, oversampling);
  if (ret != 0) {
    msg = "Pressure read FAIL! ret = " + String(ret);
  } else {
    msg = "Pressure: " + String(press / 100) + " hPa";
  }
  Serial.println(msg);
  select_channel_i2c(OLED_IIC_CH);
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(0, OLED_ROW_3, msg.c_str());



  select_channel_i2c(SHT35_IIC_CH);
  if (NO_ERROR == sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH,&(sensor_value[S_TEMP]),&(sensor_value[S_HUMIDITY])) ){
          sample_count++;
          select_channel_i2c(OLED_IIC_CH);
          u8g2.setFont(u8g2_font_profont12_tr);
          String msg = "Temperature: " + String(sensor_value[S_TEMP]) + " °C ";
          u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
          Serial.println(msg);
          msg = "Humidity: " + String(sensor_value[S_HUMIDITY]) + " % "; 
          u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
          Serial.println(msg);
          u8g2.sendBuffer();

      if (sample_count == LOG_EVERY) {
      // Format values for logfile:
      msg = /*formatTime() + "," +*/ String(sensor_value[S_TEMP]) + ", " +String(sensor_value[S_HUMIDITY]) + "\r\n"; 
      appendFile(SPIFFS, LOGFILE, msg.c_str());
      sample_count = 0;
    }
  }
    else {
      Serial.println("Sensor read failed!");
      Serial.println("");
    }

}

void prosjeci() {
  

  select_channel_i2c(SHT35_IIC_CH);
  sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &sensor_value[S_TEMP], &sensor_value[S_HUMIDITY]);

  select_channel_i2c(DPS310_IIC_CH);
  ret = Dps3xxPressureSensor.measurePressureOnce(press, oversampling);

  String msg;


  if (!isnan(sensor_value[S_TEMP]) && !isnan(sensor_value[S_HUMIDITY]) && !isnan(press)) {
    if (sensor_value[S_TEMP]< tempMin) tempMin = sensor_value[S_TEMP];
    if (sensor_value[S_TEMP] > tempMax) tempMax = sensor_value[S_TEMP];


    tempAvg = (tempAvg * readingsCount + temp) / (readingsCount + 1);

    if (sensor_value[S_HUMIDITY] < humMin) humMin = sensor_value[S_HUMIDITY];
    if (sensor_value[S_HUMIDITY] > humMax) humMax = sensor_value[S_HUMIDITY];


    humAvg = (humAvg * readingsCount + sensor_value[S_TEMP]) / (readingsCount + 1);

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

    select_channel_i2c(OLED_IIC_CH);
    msg = "TempAVG: " + String(tempAvg) + " *C ";
    u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
    msg = "HumAVG: " + String(humAvg) + " % ";
    u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
    msg = "PressAVG: " + String(pressAvg/100) + " hPa ";
    u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
    u8g2.sendBuffer();

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


// Show an alarm if any value is outside set limits:
void handleAlarms() {
  bool alarm = false;
  select_channel_i2c(OLED_IIC_CH);
  u8g2.clearBuffer();
  if (sensor_value[S_TEMP] > alarms[S_TEMP][A_HI]) {
    alarm = true;
    u8g2.drawStr(0, 42, "HIGH TEMPERATURE");
    u8g2.sendBuffer();
  } else if (sensor_value[S_TEMP] < alarms[S_TEMP][A_LO]) {
    alarm = true;
    u8g2.drawStr(0, 42, "LOW TEMPERATURE");
    u8g2.sendBuffer();
  }
  if (sensor_value[S_HUMIDITY] > alarms[S_HUMIDITY][A_HI]) {
    alarm = true;
    u8g2.drawStr(0, 56, "HIGH HUMIDITY");
    u8g2.sendBuffer();
  } else if (sensor_value[S_HUMIDITY] < alarms[S_HUMIDITY][A_LO]) { 
    alarm = true;
    u8g2.drawStr(0, 56, "LOW HUMIDITY");
    u8g2.sendBuffer();
  }
  //digitalWrite(ALARM_PIN, alarm);
  delay(1000);
}

void appendFile(fs::FS &fs, const char * path, const char * message) {
  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.print("File ");
    Serial.print(path);
    Serial.println(" not found. Attempting to create new file."); 
    file = fs.open(path, FILE_WRITE);
    if (!file) {
      Serial.print("Unable to create file ");
      Serial.println(path);
      return;
    }
    Serial.println("File created.");
  }
  if (!file.print(message)) {
    Serial.print("Failed to append to file ");
    Serial.println(path);
  }
  file.close();
};

// Reads file /path/ on filesystem /fs/ and sends it over serial port and displays on screen.
void displayFile(fs::FS &fs, U8G2& disp, const char * path) {
  select_channel_i2c(OLED_IIC_CH);
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("Failed to open file for reading!");
    return; 
  }
  String buffer;
  while (file.available()) {
    char next = file.read();
    buffer += next;
    if (next == '\n'){
      Serial.print(buffer);
      disp.clearBuffer();
      disp.setFont(u8g2_font_profont12_tr);
      disp.drawStr(0, 10, "Log file:");
      disp.drawStr(0, 28, buffer.c_str());
      disp.sendBuffer();
      buffer = ""; 
    }
  }
  file.close();
};

void initPrefs() {
  // Attempt to retrieve stored settings.
  prefs.begin("Veleri-OI-meteo", false);
  bool hasPrefs = prefs.getBool("valid", false);
  if (hasPrefs) {
    Serial.println("Saved settings found. "); String buf;
    double dbuf;
    unsigned int ibuf;
    // Read settings into a temporary variable and store if no error. ibuf = prefs.getUInt("IDMeteoStation");

              dbuf = prefs.getFloat("a_temp_lo");
                if (!isnan(dbuf)) alarms[S_TEMP][A_LO] = dbuf;
                  dbuf = prefs.getFloat("a_temp_hi");
      if (!isnan(dbuf)) alarms[S_TEMP][A_HI] = dbuf; 
        dbuf = prefs.getFloat("a_hum_lo");
          if (!isnan(dbuf)) alarms[S_HUMIDITY][A_LO] = dbuf; 
            dbuf = prefs.getFloat("a_hum_hi");
              if (!isnan(dbuf)) alarms[S_HUMIDITY][A_HI] = dbuf;
    } else {
      Serial.println("Saved settings not found, using default values.");
  } 
}

