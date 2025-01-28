#include <V5header.h>

void setup()
{
  
  DeserializationError error = deserializeJson(doc, jsonstring);
    // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  client.begin("http://aserver.example/foo?bar=1");

  if (client.GET() > 0){ // Send request and check the error code
  response = client.getString();
  }


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
  server.on("/all", handleAll); 
  AsyncCallbackJsonWebHandler* handleSetAlarmJson = new AsyncCallbackJsonWebHandler("/set_alarms", jsonAlarmHandler); 
  server.addHandler(handleSetAlarmJson); 
  AsyncCallbackJsonWebHandler* handleSetConfigJson = new AsyncCallbackJsonWebHandler("/config", jsonConfigHandler); 
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
  if (NO_ERROR == sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH,&(sensor_value[S_TEMP]),&(sensor_value[S_HUMIDITY])) ){
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
      msg = formatTime() + "," + String(sensor_value[S_TEMP]) + ", " +String(sensor_value[S_HUMIDITY]) + "\r\n"; 
      appendFile(SPIFFS, LOGFILE, msg.c_str());
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
  HTTPClient client; 
  client.setConnectTimeout(1000); 
  client.setTimeout(1000); 
  client.begin(hostname, port, "/setReading"); 
  client.POST(jsonMessage);
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
  } else if (sensor_value[S_HUMIDITY] < alarms[S_HUMIDITY][A_LO]) { 
    alarm = true;
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
    // Arduino toFloat() returns 0 if string cannot be converted to float; 
    // Can't differentiate between actual zero value and error, so we need 
    // to manually check the string.
    if (buf.length() > 0 && isdigit(buf[0])) {
        alarms[S_TEMP][A_LO] = buf.toFloat();
        confirm += "temp_lo: " + String(alarms[S_TEMP][A_LO]) + "\n"; 
      }
  }

  if (request->hasArg("a_temp_hi")) {
    buf = request->arg("a_temp_hi");
    if (buf.length() > 0 && isdigit(buf[0])) {
      alarms[S_TEMP][A_HI] = buf.toFloat();
      confirm += "temp_hi: " + String(alarms[S_TEMP][A_HI]) + "\n"; 
    }

  }

  if (request->hasArg("a_hum_lo")) {
    buf = request->arg("a_hum_lo");
    if (buf.length() > 0 && isdigit(buf[0])) {
      alarms[S_HUMIDITY][A_LO] = buf.toFloat();
      confirm += "hum_lo: " + String(alarms[S_HUMIDITY][A_LO]) + "\n"; 
    }
    
  }

  if (request->hasArg("a_hum_hi")) {
    buf = request->arg("a_hum_hi");
    if (buf.length() > 0 && isdigit(buf[0])) {
      alarms[S_HUMIDITY][A_HI] = buf.toFloat();
      confirm += "hum_hi: " + String(alarms[S_HUMIDITY][A_HI]) + "\n"; 
    }
  }

  String message = "Ok, settings recieved: " + confirm; 
  saveAlarms();
  request->send(200, "text/plain", message);
  } 
};

// Save current alarm values to permanent storage.
void saveAlarms(){
  prefs.putFloat("a_temp_lo", alarms[S_TEMP][A_LO]); 
  prefs.putFloat("a_temp_hi", alarms[S_TEMP][A_HI]); 
  prefs.putFloat("a_hum_lo", alarms[S_HUMIDITY][A_LO]); 
  prefs.putFloat("a_hum_hi", alarms[S_HUMIDITY][A_HI]);
}



// Process single alarm upper and lower bound. 
void processJsonAlarm(const JsonObject& jo) {
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
  } 
};

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
      unsigned int tmp = jo["IDMeteoStation"].as<unsigned int>(); 
      Serial.print("Setting new IDMeteostation: "); Serial.print(tmp);
      IDMeteoStation = tmp;
      prefs.putUInt("IDMeteoStation", tmp);
    } else {
      Serial.println("Invalid ID");
    } 
  }

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
  } 
};


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
};



// Send all current measurements to server in JSON format. 
void handleAll(AsyncWebServerRequest *request) {
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
    if (ibuf != 0) IDMeteoStation = ibuf; 
      buf = prefs.getString("ssid");
        if (buf != String()) ssid = buf;
          buf = prefs.getString("passphrase"); 

    if (buf != String()) wifi_pass = buf; 
      buf = prefs.getString("hostname");
        if (buf != String()) hostname = buf;
          ibuf = prefs.getUInt("server_port");
            if (ibuf != 0) server_port = ibuf;
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

// Start or restart wifi connection using saved settings. 
void initWifi() {
  int connection_attempts = 0;
  Serial.println("Using ssid = " + ssid);
  Serial.print("Passphrase = ");
  Serial.print(wifi_pass[0]);
  Serial.print("******");
  WiFi.begin(ssid.c_str(), wifi_pass.c_str());
    while (WiFi.status() != WL_CONNECTED && connection_attempts < 20) {
      delay(500);
      connection_attempts++;
      Serial.print(".");
    }
  Serial.println("");
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WiFi connected");
      Serial.println("IP address: "); 
      Serial.println(WiFi.localIP());
// Attempt to get time from NTP on successful wifi connection: initTime();
  } else {
    WiFi.disconnect();
    Serial.println("Wifi connection failed, continuing without wifi."); 
  }
};


void handleRoot(AsyncWebServerRequest *request) {
  const String part1 = "<p>Temperature: ";
  const String part2 = "&deg;C</p><p>Humidity: ";
  const String part3 = " %</p> </p> <a href=\"log\">Sensor log</a></p></body></html>"; 
  request->send(200, "text/html", www_header + part1 + String(sensor_value[S_TEMP]) + part2 + String(sensor_value[S_HUMIDITY]) + part3); 
};

// www /log page
void handleLog(AsyncWebServerRequest *request) {
  request->send(SPIFFS, "/sensor_log.csv", "text/plain"); 
};

// Get current time from the Internet NTP service
void initTime(){
    const char* ntpServer = "pool.ntp.org";
    // Time zone and daylight savings time can be set,
    // we will use UTC time so no offset.
    const long UTCOffset_sec = 0;
    const int dstOffset_sec = 0;
    configTime(UTCOffset_sec, dstOffset_sec, ntpServer);
};

// Recieve and parse commands over serial connection. 
void handleSerialCommands() {
  while (Serial.available()) {
    String incomingRow = Serial.readStringUntil('\n'); 
    Serial.println("rcvd." + incomingRow);
    if (incomingRow.equals("setSSID")) {
      Serial.println("setting new ssid"); 
      ssid = Serial.readStringUntil('\n'); 
      prefs.putString("ssid", ssid);
    } 
    else if (incomingRow.equals("setPass")) {
      Serial.println("Setting new passphrase.");
      wifi_pass = Serial.readStringUntil('\n');
      prefs.putString("passphrase", wifi_pass);
    } 
    else if (incomingRow.equals("dumpLogFile")){
      displayFile(SPIFFS, u8g2, LOGFILE);
    }
  } 
};


// Appends /message/ to file /path/ on filesystem /fs/, creating if necessary. 
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

// Returns current date and time as string or xxxx-xx-xx,xx:xx:xx if time is not set 
String formatTime(){
  // Structure used to hold time data (defined in time.h):
  tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    return String("xxxx-xx-xx,xx:xx:xx"); 
  }
  return String(timeinfo.tm_year+1900) + "-" + String(timeinfo.tm_mon+1) + "-" +String(timeinfo.tm_mday) + "," + String(timeinfo.tm_hour) + ":" + String(timeinfo.tm_min) + ":" + String(timeinfo.tm_sec);
};

String processor(const String& var){
  if(var == "VRIJEDNOST_TEMPERATURE")
    return String(sensor_value[0]);
  return String();
};
