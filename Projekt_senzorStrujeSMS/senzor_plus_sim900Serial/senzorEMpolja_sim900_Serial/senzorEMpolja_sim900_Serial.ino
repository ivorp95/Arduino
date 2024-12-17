
#include <SoftwareSerial.h>  //softwareSerial za koristenje digitalnih pinova za serijsku komunikaciju sa serial900 

#define NUMREADINGS 32
#define MAXBROJAC 30000


int updateTime = 100;

int LED0 = 13;  //interlnal led
int LED2B = 3;
int LED3G = 4;
int LED4R = 5;

int paljenje=9; //digital 9 na pin 9 sim900 za automatsko paljenje

SoftwareSerial serial900(7,8); //objekt serial za serial900(software serial na pinovima D 7 i 8)

int senseLimit = 12;  // raise this number to decrease sensitivity (up to 1023 max)
int probePin = 5;     // analog 5
int val = 0;          
int brojacZaPoruku = 0;   


int readings[NUMREADINGS];
int index = 0;
int total = 0;            
int average = 0;            

void setup() {
  digitalWrite(paljenje, HIGH);
  delay(1000);
  digitalWrite(paljenje, LOW);
  delay(10000);

  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(13, OUTPUT);

  for (int i = 0; i < NUMREADINGS; i++)
    readings[i] = 0; 

  Serial.begin(38400);
  serial900.begin(38400);


  Inicjalizacija();
  delay(200);
  pozivIvor();
  delay(1000);
  intro();
}



void loop() {

  LEDlow();
  val = analogRead(probePin);


  if (val > 0) {                             
    val = constrain(val, 1, senseLimit);   
    val = map(val, 1, senseLimit, 1, 1023); 

    total -= readings[index];
    readings[index] = val;    
    total += readings[index];
    index++;                  

    if (index >= NUMREADINGS)  
      index = 0;
      average = total / NUMREADINGS;


    if (average>=0 && average <250) {
      brojacZaPoruku++;
      showLED0();
      delay(100);
    }
      if (brojacZaPoruku==300){
        SMSivor();
        brojacZaPoruku++;
      }
        if(brojacZaPoruku>=MAXBROJAC){
          SMSivor();
          brojacZaPoruku=0;
        }
        

    if (average >=0) {
      showLED2B();
    }
      if (average > 200) {
        showLED2B();
      }
        if (average > 350) {
          showLED4R();
        }
          if (average > 600) {
            showLED3G();
          }


    Serial.println(average);
    Serial.println(brojacZaPoruku);

    Serial.readString()

    updateSerial();

    delay(updateTime);
  }
}


void Inicjalizacija(){

  Serial.println("Inicjalizacija serial900");
  delay(1000);
  serial900.println("AT"); //Handshaking with serial900
  updateSerial();
  serial900.println("AT+CSQ"); //Signal quality test, value range is 0-31 , 31 is the best
  updateSerial();
  serial900.println("AT+CCID"); //Read SIM information to confirm whether the SIM is plugged
  updateSerial();
  serial900.println("AT+CREG?"); //Check whether it has registered in the network
  updateSerial();
  serial900.println("AT+COPS?"); //print network data
  updateSerial();
}

void pozivIvor(){
  
  Serial.println("Poziv Ivor 10 sec");
  showLED3G();
  delay(2000);


  serial900.println("ATD+ +385912016999;");
  updateSerial();
  delay(10000);
  serial900.println("ATH");
  updateSerial();
}


void SMSivor(){
  Serial.println("Poruka Ivor.");
  serial900.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  serial900.println("AT+CMGS=\"+385912016999\"");
  updateSerial();
  serial900.print("Arduino Nano: 091-2016-400 Nestanak Napona na senzoru. Oprez"); 
  updateSerial();
  serial900.write(26); //asci kontrolni znak (26dec 032oct	1Ahex	00011010bin	SUBsym	&#26html;	 	Substitute)

  showLED0();
  delay(200);
  showLED4R();
  delay(200);
  showLED3G();
  delay(200);
  showLED2B();
  delay(200);
  LEDlow();
}


void SMSigor(){

  serial900.println("AT+CMGF=1");
  updateSerial();
  serial900.println("AT+CMGS=\"+385912016000\"");
  updateSerial();
  serial900.print("pozdrav Arduino. radi");
  updateSerial();
  serial900.write(26);

  showLED0();
  delay(200);
  showLED4R();
  delay(200);
  showLED3G();
  delay(200);
  showLED2B();
  delay(200);
  LEDlow();

}


void updateSerial(){
  
  delay(500);
  while (Serial.available()) {
    serial900.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(serial900.available()) {
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