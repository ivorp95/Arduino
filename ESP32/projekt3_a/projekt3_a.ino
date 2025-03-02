#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Dps3xx.h>
#include <Seeed_SHT35.h>
#include <U8g2lib.h>
#include <Wire.h>
#include <Preferences.h>
#include "FS.h"
#include <LittleFS.h>

// Wi-Fi credentials
const char* ssid = "ivors21";
const char* password = "";
uint8_t oversampling = 7;

// Sensor and I2C settings
const unsigned char TCA9548A_DEF_ADDR = 0x70; // I2C multiplexer address
const unsigned char DPS310_IIC_ADDR = 0x77;   // DPS310 address
const unsigned char SHT35_IIC_ADDR = 0x45;    // SHT35 address
const unsigned char SCLPIN = 22;              // I2C SCL pin
const unsigned char SDAPIN = 21;              // I2C SDA pin

const unsigned char DPS310_IIC_CH = 0;
const unsigned char SHT35_IIC_CH = 1;
const unsigned char OLED_IIC_CH = 2;

char* LOGFILE = "/sensor_log.csv";



int16_t ret;
String msg = "";
// Sensor objects
Dps3xx Dps3xxPressureSensor = Dps3xx();
SHT35 sht35Sensor(SCLPIN, SHT35_IIC_ADDR);

// OLED display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// Web server
AsyncWebServer server(80);

// Sensor data
float temperature = 0.0;
float humidity = 0.0;
float pressure = 0.0;

// Function to select I2C channel on the multiplexer
void selectI2CChannel(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_DEF_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

// Function to read sensor data
void readSensorData() {
  // Read DPS310 data
  selectI2CChannel(DPS310_IIC_CH); // Select DPS310 channel
  //Dps3xxPressureSensor.measureTempOnce(temperature,oversampling);
  ret = Dps3xxPressureSensor.measurePressureOnce(pressure, oversampling);
  if (ret != 0) {
    msg = "Pressure read FAIL! ret = " + String(ret);
  } else {
    msg = "Pressure: " + String(pressure / 100) + " hPa";
  }
  //Dps3xxPressureSensor.measurePressureOnce(pressure,oversampling);

  // Read SHT35 data
  selectI2CChannel(SHT35_IIC_CH); // Select SHT35 channel
  sht35Sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temperature, &humidity);
}

// HTML template for the web server
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css">
  <style>
    html { font-family: Arial; display: inline-block; margin: 0px auto; text-align: center; }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .labels { font-size: 1.5rem; vertical-align: middle; padding-bottom: 15px; }
  </style>
</head>
<body>
  <h2>ESP32 Sensor Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="labels">Temperature</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <p>
    <i class="fas fa-tachometer-alt" style="color:#ff0000;"></i> 
    <span class="labels">Pressure</span>
    <span id="pressure">%PRESSURE%</span>
    <sup class="units">hPa</sup>
  </p>
</body>
<script>
setInterval(function() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000);

setInterval(function() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000);

setInterval(function() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pressure").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pressure", true);
  xhttp.send();
}, 10000);
</script>
</html>)rawliteral";

// Function to replace placeholders in HTML with sensor data
String processor(const String& var) {
  if (var == "TEMPERATURE") {
    return String(temperature);
  } else if (var == "HUMIDITY") {
    return String(humidity);
  } else if (var == "PRESSURE") {
    return String(pressure / 100);
  }
  return String();
}


void displayFile(fs::FS &fs, U8G2& disp, const char * path) {
  selectI2CChannel(OLED_IIC_CH);
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
}

void setup() {

  selectI2CChannel(OLED_IIC_CH);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);//8px font
  msg="PROJEKTNI ZADATAK";
  u8g2.drawStr(0, 15, msg.c_str());
  u8g2.sendBuffer();


  Serial.begin(115200);


  Wire.begin(SDAPIN, SCLPIN);

  selectI2CChannel(DPS310_IIC_CH);
  Dps3xxPressureSensor.begin(Wire);


  selectI2CChannel(SHT35_IIC_CH);
  if (sht35Sensor.init())
    Serial.println("SHT35 init failed!");

  // Initialize OLED
  u8g2.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Initialize LittleFS
  if (!LittleFS.begin()) {
    Serial.println("LittleFS initialization failed!");
  }

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  // Routes for sensor data
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", String(temperature).c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", String(humidity).c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/plain", String(pressure / 100).c_str()); // Convert Pa to hPa
  });

  // Start server
  server.begin();

  displayFile(LittleFS, u8g2, LOGFILE);
}

void loop() {
  // Read sensor data
  int16_t ret;
  readSensorData();

  // Display sensor data on OLED
  selectI2CChannel(OLED_IIC_CH);
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont12_tr);
  u8g2.drawStr(0, 15, ("Temp: " + String(temperature) + " C").c_str());
  u8g2.drawStr(0, 30, ("Humidity: " + String(humidity) + " %").c_str());
  u8g2.drawStr(0, 45, ("Pressure: " + String(pressure / 100.0) + " hPa").c_str());
  u8g2.sendBuffer();

  // Log sensor data to LittleFS (optional)
  String logEntry = String(millis()) + "," + String(temperature) + "," + String(humidity) + "," + String(pressure / 100) + "\n";
  File file = LittleFS.open("/sensor_log.csv", "a");
  if (file) {
    file.print(logEntry);
    file.close();
  }

  // Delay between readings
  delay(5000);
}



