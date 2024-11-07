

//#include <sim900.h>          //library za koristenje SIM900 modula za mobilne mreze ili direktno AT naredbe
#include <SoftwareSerial.h>  //softwareSerial za koristenje digitalnih pinova za serijsku komunikaciju sa sim900 

#define NUMREADINGS 32
int updateTime = 100;

SoftwareSerial serial900(7,8); //objekt serial za sim900(software serial na pinovima D 7 i 8)

int senseLimit = 12;  // raise this number to decrease sensitivity (up to 1023 max)
int probePin = 5;     // analog 5
int val = 0;          // reading from probePin
int trueVal = 0;    


int LED0 = 13;  //interlnal led
int LED2B = 3;
int LED3G = 4;
int LED4R = 5;


int readings[NUMREADINGS];
int index = 0;
int total = 0;            
int average = 0;            




void setup() {
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(13, OUTPUT);


  Serial.begin(38400);

  serial900.begin(38400);

  Serial.println("inicjalizacija sim900");
  delay(1000);

  serial900.println("AT"); //Handshaking with SIM900
  updateSerial();
  serial900.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  serial900.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  serial900.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  serial900.println("AT+COPS?"); //print network data
  updateSerial();

  // poziv 10 sekundi i prekid
  serial900.println("ATD+ +385912016999;");
  updateSerial();
  delay(10000);
  serial900.println("ATH");
  updateSerial();


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
    readings[index] = val;    
    total += readings[index];
    index++;                  

    if (index >= NUMREADINGS)  
      index = 0;

    average = total / NUMREADINGS;

    if (average > 0) {
      showLED0();
      
    }
    if (average > 200) {
      showLED2B();
    }

    if (average > 50 && average <100) {
      //SMSivor();
    }

    if (average > 450) {
      showLED4R();
    }

    if (average > 700) {
      showLED3G();
    }

    Serial.println(average);

    //Serial.println(trueVal);

    //updateSerial();

    delay(updateTime);
  }
}

void SMSivor(){

  serial900.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  serial900.println("AT+CMGS=\"+385912016999\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  serial900.print("pozdrav Arduino. radi"); //text content
  updateSerial();
  serial900.write(26); //asci za ctrl+z

}


void SMSigor(){

  serial900.println("AT+CMGF=1");
  updateSerial();
  serial900.println("AT+CMGS=\"+385912016000\"");
  updateSerial();
  serial900.print("pozdrav Arduino. radi");
  updateSerial();
  serial900.write(26);

}

void updateSerial()
{
  delay(100);
  while (Serial.available()) 
  {
    serial900.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(serial900.available()) 
  {
    Serial.write(serial900.read());//Forward what Software Serial received to Serial Port
  }
}




void showLED2B() {
  LEDlow();
  digitalWrite(LED2B, HIGH);
}

void showLED3G() {
  LEDlow();
  digitalWrite(LED3G, HIGH);
}

void showLED4R() {
  LEDlow();
  digitalWrite(LED4R, HIGH);
}

void showLED0() {
  LEDlow();
  digitalWrite(LED0, HIGH);
}

void LEDlow() {
  digitalWrite(LED2B, LOW);
  digitalWrite(LED3G, LOW);
  digitalWrite(LED4R, LOW);
  digitalWrite(LED0, LOW);
}

void intro() {
  showLED2B();
  delay(5000);
  LEDlow();
  showLED3G();
  delay(500);
  LEDlow();
  showLED4R();
  delay(500);
  LEDlow();
}