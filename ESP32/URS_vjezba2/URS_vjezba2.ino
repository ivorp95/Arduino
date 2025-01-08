


const int ldrPin = A0;           // LDR analogni pin
const int potPin = A1;           // Potenciometar analogni pin
const int ledPin = 9;            // LED PWM pin
const int touchPin = 2;          // Touch senzor digitalni pin

// Varijable za režime rada
enum Mode {AUTO, MANUAL, OFF};
Mode currentMode = AUTO;

// Kalibracijski parametri
const float referenceVoltage = 5.0; 
const int analogMax = 1023; 

const int ledMinBrightness = 50;

// Touch senzor stanje
bool touchState = false;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(touchPin, INPUT_PULLUP);
}

void loop() {
  if (digitalRead(touchPin) == LOW) {
    touchState = !touchState;  // Promjena stanja na dodir
    delay(500);  // Debouncing
  }
  
  // Odabir načina rada
  if (touchState) {
    switchMode();
  }
  
  // Izvršavanje trenutnog načina rada
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
  
  delay(200); // Kratka pauza između iteracija
}

// Funkcija za promjenu režima rada
void switchMode() {
  static int modeIndex = 0;
  modeIndex = (modeIndex + 1) % 3; // 0: AUTO, 1: MANUAL, 2: OFF
  
  if (modeIndex == 0) currentMode = AUTO;
  else if (modeIndex == 1) currentMode = MANUAL;
  else if (modeIndex == 2) currentMode = OFF;
  
  Serial.print("Trenutni način rada: ");
  if (currentMode == AUTO) Serial.println("AUTO");
  if (currentMode == MANUAL) Serial.println("MANUAL");
  if (currentMode == OFF) Serial.println("OFF");
}

// AUTO način rada: LED se prilagođava intenzitetu svjetla
void autoMode() {
  int analogValue = analogRead(ldrPin);
  float voltage = (analogValue / float(analogMax)) * referenceVoltage;
  float lux = 500.0 / (referenceVoltage - voltage) - 50.0;  // Kalibracija potrebna
  
  // Obrnuto proporcionalna kontrola svjetline
  int brightness = map(analogValue, 0, analogMax, 255, ledMinBrightness);
  analogWrite(ledPin, brightness);
  
  Serial.print("AUTO -> Analog: ");
  Serial.print(analogValue);
  Serial.print(" | Lux: ");
  Serial.print(lux);
  Serial.print(" | LED svjetlina: ");
  Serial.println(brightness);
}

// MANUAL način rada: LED intenzitet se kontrolira potenciometrom
void manualMode() {
  int potValue = analogRead(potPin);
  int brightness = map(potValue, 0, analogMax, ledMinBrightness, 255);
  analogWrite(ledPin, brightness);
  
  Serial.print("MANUAL -> Potenciometar: ");
  Serial.print(potValue);
  Serial.print(" | LED svjetlina: ");
  Serial.println(brightness);
}

// OFF način rada: LED je ugašen
void offMode() {
  analogWrite(ledPin, 0); // LED je ugašena
  Serial.println("OFF -> LED je ugašena.");
}