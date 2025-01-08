// Pinovi za povezivanje
const int sensorPin = 32;       // Pin za fotootpornik (LDR)
const int ledPin = 5;           // Pin za LED
const int potPin = 33;          // Pin za potenciometar
const int touchPin = 2;         // Pin za touch senzor

// Varijable za mjerenje
int sensorValue = 0;            // Vrijednost očitana sa fotootpornika
int luxValue = 0;               // Vrijednost u lux-ima
int ledBrightness = 0;          // Intenzitet LED-a
int potValue = 0;               // Vrijednost s potenciometra

// Definicija režima rada
enum Mode {AUTO, MANUAL, OFF};  // Režimi rada
Mode currentMode = AUTO;        // Početni režim je AUTO

void setup() {
  pinMode(ledPin, OUTPUT);      // LED pin je izlaz
  pinMode(touchPin, INPUT);     // Touch pin je ulaz
  pinMode(potPin, INPUT);       // Potenciometar pin je ulaz
  Serial.begin(9600);           // Pokrećemo serijsku komunikaciju
}

void loop() {
  if (digitalRead(touchPin) == HIGH) {
    currentMode = static_cast<Mode>((currentMode + 1) % 3);  // Prebacivanje režima: AUTO -> MANUAL -> OFF
    delay(500);  // Debouncing
  }

  // Režim rada ovisi o trenutnom odabiru
  switch (currentMode) {
    case AUTO:
      autoMode();  // Automatski upravlja LED prema svjetlu u prostoriji
      break;
      
    case MANUAL:
      manualMode();  // Ručna kontrola intenziteta LED-a pomoću potenciometra
      break;
      
    case OFF:
      offMode();  // LED je ugašen
      break;
  }

  delay(100);  // Kratko kašnjenje za stabilizaciju
}

// Funkcija za automatski režim
void autoMode() {
  // Očitavamo vrijednost s fotootpornika
  sensorValue = analogRead(sensorPin);
  
  // Mapiramo očitane vrijednosti u lux (prema kalibraciji)
  luxValue = map(sensorValue, 0, 1023, 0, 1000);  // Ovdje prilagodite prema stvarnim vrijednostima

  // Obrnuto proporcionalna veza za jačinu LED-a: manji lux => jača LED
  ledBrightness = map(luxValue, 0, 1000, 255, 0);  // 0 = LED maksimalno svijetli, 255 = LED isključen
  analogWrite(ledPin, ledBrightness);  // Postavljanje intenziteta LED-a
  Serial.print("Auto");
  // Ispisivanje vrijednosti lux
  Serial.print("Lux: ");
  Serial.println(luxValue);
}

// Funkcija za manualni režim
void manualMode() {
  // Očitavamo vrijednost s potenciometra
  potValue = analogRead(potPin);
  
  // Mapiramo potenciometar na vrijednost između 0 i 255 za LED
  ledBrightness = map(potValue, 0, 1023, 0, 255);  // Postavljanje intenziteta prema potenciometru
  analogWrite(ledPin, ledBrightness);  // Postavljanje intenziteta LED-a
  Serial.print("Manual");
  // Ispisivanje vrijednosti s potenciometra
  Serial.print("Potenciometar: ");
  Serial.println(potValue);
}

// Funkcija za off režim
void offMode() {
  // LED je ugašen
  analogWrite(ledPin, 0);  // LED je isključen
  Serial.print("Off");
  // Ispisivanje poruke o gašenju LED-a
  Serial.println("LED je ugašen.");
}