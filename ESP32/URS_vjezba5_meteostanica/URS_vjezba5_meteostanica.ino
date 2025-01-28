#include <cmath> 
#include <cstdlib> 
#define err_t sht_err_t 
#include <Seeed_SHT35.h> 
#undef err_t
#include <U8g2lib.h> 
#include <Wire.h>
// Persistent storage related headers:
#include <Preferences.h>

#include "FS.h"
#include <LittleFS.h>
#define SPIFFS LittleFS 


#include <WiFi.h>
#include <AsyncTCP.h>
#include <AsyncJson.h> 
#include <ArduinoJson.h> 
#include <ESPAsyncWebServer.h> 
#include <HTTPClient.h> 
#include <time.h>
#include <WiFiClient.h>
#include <ESPmDNS.h>


#define FORMAT_SPIFFS_IF_FAILED false
Preferences prefs;
AsyncWebServer server(80);
String ssid = "DEFAULT_SSID";
String wifi_pass = "default_passphrase";

// Host name of the app server:
String hostname = "192.168.5.24";
unsigned int server_port = 3000;
unsigned int IDMeteoStation = 0;
const unsigned char SCLPIN = 22;
const unsigned char SHT35_IIC_ADDR = 0x45; 
SHT35 sensor(SCLPIN, SHT_IIC_ADDR);
const int ALARM_PIN = 4;  // Alarm LED pin
const char* LOGFILE = "/sensor_log.csv";
// Log every half hour (1800 seconds):
int sample_count = 0;
const int LOG_EVERY  = 1800;
// Init display:
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);


// Sensor value storage:
enum Sensors {S_TEMP, S_HUMIDITY};  // Sensor IDs
enum AlarmLimit {A_LO, A_HI};
const int num_sensors = 2;
float sensor_value[2];
// Default alarm values:
float alarms[num_sensors][2] = {{18.0f, 23.0f}, {20.0f, 60.0f}};
unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
// WWW root
const String www_header = "<!DOCTYPE html><html><head><title>Veleri-OI-meteo station</title><style>body {background-color: white;text-align: center;color: black;font- family: Arial, Helvetica, sans-serif;}</style></head><body><h1>Veleri-OI-meteo station</h1>";
  void handleRoot(AsyncWebServerRequest *request) {
  const String part1 = "<p>Temperature: ";
  const String part2 = "&deg;C</p><p>Humidity: ";
  const String part3 = " %</p> </p> <a href=\"log\">Sensor log</a></p></body></html>"; 
  request->send(200, "text/html", www_header + part1 + String(sensor_value[S_TEMP]) + part2 + String(sensor_value[S_HUMIDITY]) + part3); 
  }



void setup()
{
  pinMode(ALARM_PIN, OUTPUT);
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_profont12_tr);
  if (sensor.init())
    Serial.println("Sensor init failed!");

  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  // Restore saved settings
  initPrefs();
  // Start wifi connection:
  initWifi();
  // Server initialization:
  server.on("/", handleRoot);
  server.on("/log", handleLog);


// REST API initialization:
  server.on("/set_alarm", HTTP_GET, handleSetAlarm); 
  server.on("/all", handleAll); AsyncCallbackJsonWebHandler* handleSetAlarmJson = new
  AsyncCallbackJsonWebHandler("/set_alarms", jsonAlarmHandler); 
  server.addHandler(handleSetAlarmJson); AsyncCallbackJsonWebHandler* handleSetConfigJson = new
  AsyncCallbackJsonWebHandler("/config", jsonConfigHandler); 
  server.addHandler(handleSetConfigJson);
  server.begin();


  // Init mDNS respornder
  if (!MDNS.begin("meteostation")) {
    Serial.println("Error setting up MDNS responder!");
    }

  Serial.println("mDNS responder started");
  delay(1000);
  // Show log file at startup:
  displayFile(SPIFFS, u8g2, LOGFILE);
  previousMillis = millis();
}



void loop()
{
  currentMillis = millis();
  // Read sensors and process new reading if
  // more than one second elapsed since last reading.
  if (currentMillis - previousMillis >= 1000) {
handleSensors();
handleAlarms();
previousMillis = currentMillis; // Remember time of this reading.
}
// Check if there are incoming commands over serial port:
  handleSerialCommands();
  // Try reconnecting if no wifi:
  if (WiFi.status() != WL_CONNECTED) {
    initWifi();
  }
}



// Read, display and log sensor values.
void handleSensors() {
  if (NO_ERROR == sensor.read_meas_data_single_shot(
        HIGH_REP_WITH_STRCH,
&(sensor_value[S_TEMP]),
&(sensor_value[S_HUMIDITY])) ){
sample_count++;
u8g2.clearBuffer();
u8g2.setFont(u8g2_font_profont12_tr);
String msg = "Temperature: " + String(sensor_value[S_TEMP]) + " °C ";
u8g2.drawStr(0, 10, msg.c_str());
Serial.println(msg);
msg = "Humidity: " + String(sensor_value[S_HUMIDITY]) + " % "; u8g2.drawStr(0, 28, msg.c_str());
Serial.println(msg);
u8g2.sendBuffer();
try {
      sendCurrentMeasurements(hostname, server_port);
    } catch (...) {
      Serial.print("Error sending measurements to ");
      Serial.println(hostname);
    }
    if (sample_count == LOG_EVERY) {
      // Format values for logfile:
      msg = formatTime() + "," + String(sensor_value[S_TEMP]) + ", " +
String(sensor_value[S_HUMIDITY]) + "\r\n"; appendFile(SPIFFS, LOGFILE, msg.c_str());
      sample_count = 0;
    }
}
else {
    Serial.println("Sensor read failed!");
    Serial.println("");
  }
}



// Sends current measurements to configured server.
void sendCurrentMeasurements(String hostname, unsigned int port) {
  const int capacity = 1024;
  StaticJsonDocument<capacity> doc;
  for (int i = 0; i < num_sensors; i++) {
doc[i]["IDMeteoStation"] = IDMeteoStation; doc[i]["IDSensor"] = i;
doc[i]["value"] = sensor_value[i];
}
String jsonMessage;
serializeJson(doc, jsonMessage);
HTTPClient client; client.setConnectTimeout(1000); client.setTimeout(1000); client.begin(hostname, port, "/setReading"); client.POST(jsonMessage);
client.end();
}


// Show an alarm if any value is outside set limits:
void handleAlarms() {
  bool alarm = false;
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
} else if (sensor_value[S_HUMIDITY] < alarms[S_HUMIDITY][A_LO]) { alarm = true;
u8g2.drawStr(0, 56, "LOW HUMIDITY");
u8g2.sendBuffer();
}
  digitalWrite(ALARM_PIN, alarm);
}



// Handler for /set_alarm path. Allows configuration of alarms using parameter-value pairs.
// /set_alarm?param=value&param2=value2...
void handleSetAlarm(AsyncWebServerRequest *request) {
  Serial.println("set alarm request parameters:");
  int params = request->params();
  String buf;
  String confirm;
if (params < 1) {
Serial.println("missing arguments"); request->send(400, "text/plain", "missing arguments");
} else {
  if (request->hasArg("a_temp_lo")) {
buf = request->arg("a_temp_lo");
// Arduino toFloat() returns 0 if string cannot be converted to float; // Can't differentiate between actual zero value and error, so we need // to manually check the string.
if (buf.length() > 0 && isdigit(buf[0])) {
      alarms[S_TEMP][A_LO] = buf.toFloat();
confirm += "temp_lo: " + String(alarms[S_TEMP][A_LO]) + "\n"; }
  }
  if (request->hasArg("a_temp_hi")) {
    buf = request->arg("a_temp_hi");
    if (buf.length() > 0 && isdigit(buf[0])) {
      alarms[S_TEMP][A_HI] = buf.toFloat();
    }
confirm += "temp_hi: " + String(alarms[S_TEMP][A_HI]) + "\n"; }
  if (request->hasArg("a_hum_lo")) {
    buf = request->arg("a_hum_lo");
    if (buf.length() > 0 && isdigit(buf[0])) {
      alarms[S_HUMIDITY][A_LO] = buf.toFloat();
    }
confirm += "hum_lo: " + String(alarms[S_HUMIDITY][A_LO]) + "\n"; }
  if (request->hasArg("a_hum_hi")) {
    buf = request->arg("a_hum_hi");
    if (buf.length() > 0 && isdigit(buf[0])) {
      alarms[S_HUMIDITY][A_HI] = buf.toFloat();
confirm += "hum_hi: " + String(alarms[S_HUMIDITY][A_HI]) + "\n"; }
}
String message = "Ok, settings recieved: " + confirm; saveAlarms();
request->send(200, "text/plain", message);
} };
// Save current alarm values to permanent storage.
void saveAlarms(){
prefs.putFloat("a_temp_lo", alarms[S_TEMP][A_LO]); prefs.putFloat("a_temp_hi", alarms[S_TEMP][A_HI]); prefs.putFloat("a_hum_lo", alarms[S_HUMIDITY][A_LO]); prefs.putFloat("a_hum_hi", alarms[S_HUMIDITY][A_HI]);
}



// Process single alarm upper and lower bound. void processJsonAlarm(const JsonObject& jo) {
  int id = jo["IDSensor"].as<int>();
  if (id < 0 || id >= num_sensors)
    return;
  float tmp;
  if (jo.containsKey("hi_value")) {
tmp = jo["hi_value"].as<float>(); Serial.print("Sensor"); Serial.print(id);
Serial.print(" new high alarm value:"); Serial.println(tmp);
    alarms[id][A_HI] = tmp;
  }
  if (jo.containsKey("lo_value")) {
    tmp = jo["lo_value"].as<float>();
    Serial.print("Sensor");
    Serial.print(id);
    Serial.print(" new low alarm value:");
    Serial.println(tmp);
    alarms[id][A_LO] = tmp;
} }
// Handler for /set_alarms path; Process a list of alarm bounds.
void jsonAlarmHandler(AsyncWebServerRequest * request, JsonVariant & json) {
  Serial.print("handling /set_alarms");
  JsonArray arr = json.as<JsonArray>();
  JsonObject jsonObj = json.as<JsonObject>();
  if (arr) {
    Serial.println("Array arguments.");
    for (int i = 0; i < arr.size(); i++) {
      processJsonAlarm(arr[i]);
    }
  } else if (jsonObj) {
    processJsonAlarm(jsonObj);
} else {
    request->send(400, "bad request");
  }
  saveAlarms();
  request->send(200, "OK");
}



// Process single configuration setting.
void processJsonConfig(const JsonObject& jo) {
if (jo.containsKey("IDMeteoStation")) {
if (jo["IDMeteoStation"].is<unsigned int>()) {
unsigned int tmp = jo["IDMeteoStation"].as<unsigned int>(); Serial.print("Setting new IDMeteostation: "); Serial.print(tmp);
IDMeteoStation = tmp;
      prefs.putUInt("IDMeteoStation", tmp);
    } else {
      Serial.println("Invalid ID");
} }
  if (jo.containsKey("hostname")) {
    String tmp = jo["hostname"];
    if (tmp != String()) {
      Serial.print("New server hostname: ");
      Serial.println(tmp);
      hostname = tmp;
      prefs.putString("hostname", hostname);
} else {
      Serial.println("Invalid hostname");
    }
  }
  if (jo.containsKey("port")) {
    if (jo["port"].is<unsigned int>()) {
      unsigned int tmp = jo["port"].as<unsigned int>();
      Serial.print("Setting new port: ");
      Serial.print(tmp);
      server_port = tmp;
      prefs.putUInt("server_port", tmp);
} else {
      Serial.println("Invalid port.");
    }
} }
// Handler for /config path; Process configuration options.
void jsonConfigHandler(AsyncWebServerRequest * request, JsonVariant & json) {
  Serial.print("handling /config");
  JsonArray arr = json.as<JsonArray>();
  JsonObject jsonObj = json.as<JsonObject>();
  if (arr) {
Serial.println("Array arguments.");
    for (int i = 0; i < arr.size(); i++) {
      processJsonConfig(arr[i]);
    }
  } else if (jsonObj) {
    processJsonConfig(jsonObj);
  } else {
    request->send(400, "bad request");
}
  request->send(200, "OK");
}



// Send all current measurements to server in JSON format. void handleAll(AsyncWebServerRequest *request) {
  const int capacity = 1024;
  StaticJsonDocument<capacity> doc;
  JsonObject obj1 = doc.createNestedObject();
  obj1["IDSensor"] = 1;
  obj1["value"] = sensor_value[S_TEMP];
  obj1["Type"] = "SHT35";
  obj1["Measurement"] = "Temperature";
  obj1["unit"] = "centigrade";
  JsonObject obj2 = doc.createNestedObject();
  obj2["IDSensor"] = 2;
  obj2["value"] = sensor_value[S_HUMIDITY];
  obj2["Type"] = "SHT35";
  obj2["Measurement"] = "Relative humidity";
  obj2["unit"] = "Percentage";
  String output;
  serializeJson(doc, output);
  request->send(200, "application/json", output);
}




void initPrefs() {
  // Attempt to retrieve stored settings.
  prefs.begin("Veleri-OI-meteo", false);
  bool hasPrefs = prefs.getBool("valid", false);
  if (hasPrefs) {
Serial.println("Saved settings found. "); String buf;
double dbuf;
unsigned int ibuf;
// Read settings into a temporary variable and store if no error. ibuf = prefs.getUInt("IDMeteoStation");
if (ibuf != 0) IDMeteoStation = ibuf; buf = prefs.getString("ssid");
if (buf != String()) ssid = buf;
buf = prefs.getString("passphrase"); 
if (buf != String()) wifi_pass = buf; buf = prefs.getString("hostname");
    if (buf != String()) hostname = buf;
    ibuf = prefs.getUInt("server_port");
    if (ibuf != 0) server_port = ibuf;
    dbuf = prefs.getFloat("a_temp_lo");
    if (!isnan(dbuf)) alarms[S_TEMP][A_LO] = dbuf;
    dbuf = prefs.getFloat("a_temp_hi");
if (!isnan(dbuf)) alarms[S_TEMP][A_HI] = dbuf; dbuf = prefs.getFloat("a_hum_lo");
if (!isnan(dbuf)) alarms[S_HUMIDITY][A_LO] = dbuf; dbuf = prefs.getFloat("a_hum_hi");
if (!isnan(dbuf)) alarms[S_HUMIDITY][A_HI] = dbuf;
} else {
Serial.println("Saved settings not found, using default values.");
} }
