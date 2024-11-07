

#include <sim900.h>          //library za koristenje SIM900 modula za mobilne mreze
#include <SoftwareSerial.h>  //softwareSerial za koristenje digitalnih pinova kao serijska komunikacija sim900

#define NUMREADINGS 32

SoftwareSerial serial900(7,8); //objekt serial za sim900

int senseLimit = 12;  // raise this number to decrease sensitivity (up to 1023 max)
int probePin = 5;     // analog 5
int val = 0;          // reading from probePin
int trueVal = 0;


int LED0 = 13;  //interlnal led
int LED2 = 3;   //PLAVA
int LED3 = 4;   //ZELENA
int LED4 = 5;   //CRVENA


// variables for smoothing

int readings[NUMREADINGS];  // the readings from the analog input
int index = 0;              // the index of the current reading
int total = 0;              // the running total
int average = 0;            // final average of the probe reading

//CHANGE THIS TO affect the speed of the updates for numbers. Lower the number the faster it updates.
int updateTime = 60;


void setup() {
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(13, OUTPUT);

  serial900.begin(38400);
  serial900.println("AT"); //Handshaking with SIM900
  updateSerial();
  serial900.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  serial900.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  serial900.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();


  Serial.begin(38400);
  Serial.println("inicjalizacija sim900")

  for (int i = 0; i < NUMREADINGS; i++)
    readings[i] = 0;
  intro();
}



void loop() {
  LEDlow();
  val = analogRead(probePin);  // take a reading from the probe
  trueVal = val;

  if (val > 0) {                             // if the reading isn't zero, proceed
    val = constrain(val, 1, senseLimit);     // turn any reading higher than the senseLimit value into the senseLimit value
    val = map(val, 1, senseLimit, 1, 1023);  // remap the constrained value within a 1 to 1023 range


    total -= readings[index];  // subtract the last reading
    readings[index] = val;     // read from the sensor
    total += readings[index];  // add the reading to the total
    index++;                   // advance to the next index

    if (index >= NUMREADINGS)  // if we're at the end of the array...
      index = 0;

    average = total / NUMREADINGS;

    if (average > 0) {
      showLED0();
    }
    if (average > 250) {
      showLED2();
    }

    if (average > 450) {
      showLED4();
    }

    if (average > 700) {
      showLED3();
    }

    //Serial.println(average);

    //Serial.println(trueVal);

    updateSerial();

    delay(updateTime);
  }
}



void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    serial900.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(serial900.available()) 
  {
    Serial.write(serial900.read());//Forward what Software Serial received to Serial Port
  }
}




void showLED2() {
  LEDlow();
  digitalWrite(LED2, HIGH);
}

void showLED3() {
  LEDlow();
  digitalWrite(LED3, HIGH);
}

void showLED4() {
  LEDlow();
  digitalWrite(LED4, HIGH);
}

void showLED0() {
  LEDlow();
  digitalWrite(LED0, HIGH);
}

void LEDlow() {
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED0, LOW);
}

void intro() {
  showLED2();
  delay(5000);
  LEDlow();
  showLED3();
  delay(500);
  LEDlow();
  showLED4();
  delay(500);
  LEDlow();
}