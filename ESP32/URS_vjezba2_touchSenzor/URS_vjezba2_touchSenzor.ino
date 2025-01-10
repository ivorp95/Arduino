

const int sensorPin = 32;       // Pin za fotootpornik (LDR)
const int ledPin = 5;
const int potPin = 33;

const int touchPin = T0; //GPIO4      touch sensor spojiti na gpio4
bool dodir = false;
bool zadnjeStanjeDodira=false;
int vrDodira=0;
const int pragDodira=30;


int sensorValue = 0;
int luxValue = 0;
int ledBrightness = 0;
int potValue = 0;

enum Mode {AUTO, MANUAL, OFF};
Mode currentMode = AUTO;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(touchPin, INPUT);
  pinMode(potPin, INPUT);
  Serial.begin(9600);
}

void loop() {

  vrDodira=touchRead(touchPin);
  dodir=vrDodira<pragDodira;

  if (dodir==true && zadnjeStanjeDodira==false){
  Serial.println(dodir);
  Serial.println(vrDodira);
  currentMode = static_cast<Mode>((currentMode + 1) % 3);
  delay(500);
  }

  switch (currentMode) {
    case AUTO:
      autoMode(); 
      break;
      
    case MANUAL:
      manualMode();
      break;
      
    case OFF:
      offMode();
      break;
  }

  delay(100);
  
  zadnjeStanjeDodira=dodir;
}


void autoMode() {
  sensorValue = analogRead(sensorPin);
  
  luxValue = map(sensorValue, 0, 4095, 0, 1000);

  ledBrightness = map(luxValue, 0, 1000, 255, 0);  
  analogWrite(ledPin, ledBrightness);
  Serial.print("Auto");
  Serial.print("Lux: ");
  Serial.println(luxValue);
}

void manualMode() {
  potValue = analogRead(potPin);
  
  ledBrightness = map(potValue, 0, 4095, 0, 255);
  analogWrite(ledPin, ledBrightness);
  Serial.print("Manual");
  Serial.print("Potenciometar: ");
  Serial.println(potValue);
}

void offMode() {
  analogWrite(ledPin, 0);
  Serial.print("Off");
  Serial.println("LED je ugašen.");
}