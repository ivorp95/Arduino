#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Wire.h>
#include "Seeed_SHT35.h"
#include <Dps3xx.h>
#include <U8g2lib.h>

// Replace with your network credentials
const char* ssid = "ivors21";
const char* password = "";

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

Dps3xx Dps3xxPressureSensor = Dps3xx();
SHT35 sensor(SCLPIN, SHT35_IIC_ADDR);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);


void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  select_channel_i2c(OLED_IIC_CH);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);//8px font
  String msg="Vjezba 4";
  u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
  u8g2.sendBuffer();

  select_channel_i2c(DPS310_IIC_CH);
  Dps3xxPressureSensor.begin(Wire);

  select_channel_i2c(SHT35_IIC_CH);
  if (sensor.init())
    Serial.println("SHT35 init failed!");
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html[], processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ocitajTemp().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ocitajVlaz().c_str());
  });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", ocitajPritisak().c_str());
  });

  // Start server
  server.begin();
}
 
void loop(){
  
}


void select_channel_i2c(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_DEF_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

String ocitajTemp() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
  select_channel_i2c(DPS310_IIC_CH);
  float t;
  Dps3xxPressureSensor.measureTempOnce(t, oversampling); 
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(t)) {    
    Serial.println("Neuspjesno ocitanje sa senzora temperature!");
    return "--";
  }
  else {
    Serial.println(t);
    return String(t);
  }
}


String ocitajVlaz() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  select_channel_i2c(SHT35_IIC_CH);
  float h;
  if (NO_ERROR == sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH,&(sensor_value[h])) );
  if (isnan(h)) {
    Serial.println("Neuspjesno ocitanje sa senzora vlage!");
    return "--";
  }
  else {
    Serial.println(h);
    return String(h);
  }
}

String ocitajPritisak() {
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  // Read temperature as Celsius (the default)
    select_channel_i2c(DPS310_IIC_CH);
  float p;
  Dps3xxPressureSensor.measurePressureOnce(p, oversampling); // prilagodit na nase senzore
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //float t = dht.readTemperature(true);
  // Check if any reads failed and exit early (to try again).
  if (isnan(p)) {    
    Serial.println("Neuspjesno ocitanje sa senzora pritiska!");
    return "--";
  }
  else {
    Serial.println(p);
    return String(p);
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 sensor Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="dht-labels">Temperatura</span> 
    <span id="temperatura">%TEMPERATURA%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Vlaznost</span>
    <span id="vlaznost">%VLAZNOST%</span>
    <sup class="units">&percnt;</sup>
  </p>
   <p>
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span class="dht-labels">Pritisak</span>
    <span id="pritisak">%PRITISAK%</span>
    <sup class="units">&percnt;</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperatura").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperatura", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("vlaznost").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/vlaznost", true);
  xhttp.send();
}, 10000 ) ;
</script>


setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("pritisak").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/pritisak", true);
  xhttp.send();
}, 10000 ) ;
</html>)rawliteral";


// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return ocitajTemp();
  }
  else if(var == "HUMIDITY"){
    return ocitajVlaz();
  }
  else {
    return ocitajPritisak();
  }
  return String();
}