
#define NUMREADINGS 32        // raise this number to increase data smoothing

int senseLimit = 10; // raise this number to decrease sensitivity (up to 1023 max)
int probePin = 5; // analog 5
int val = 0; // reading from probePin
int percentile=0;

int led0=13;    //interlnal led


int LED1 = 2;   // connections
int LED2 = 3;   // to
int LED3 = 4;   // 7-segment
int LED4 = 5;   // display
int LED5 = 6;
int LED6 = 7;
int LED7 = 8;

// variables for smoothing

int readings[NUMREADINGS];                // the readings from the analog input
int index = 0;                            // the index of the current reading
int total = 0;                            // the running total
int average = 0;                          // final average of the probe reading

//CHANGE THIS TO affect the speed of the updates for numbers. Lower the number the faster it updates.
int updateTime = 60;

void setup() {

  pinMode(2, OUTPUT);  // specify Display outputs
  pinMode(3, OUTPUT); 
  pinMode(4, OUTPUT); 
  pinMode(5, OUTPUT); 
  pinMode(6, OUTPUT); 
  pinMode(7, OUTPUT); 
  pinMode(8, OUTPUT); 
  pinMode(9, OUTPUT); 

  pinMode(13, OUTPUT); 


  Serial.begin(38400);  // initiate serial connection for debugging/etc

  for (int i = 0; i < NUMREADINGS; i++)
    readings[i] = 0;                      // initialize all the readings to 0

    intro(); //Runs the intro:'EFD0123456789'
}

void loop() {
  LEDlow();
  val = analogRead(probePin);  // take a reading from the probe

  if(val >= 1){                // if the reading isn't zero, proceed
    val = constrain(val, 1, senseLimit);  // turn any reading higher than the senseLimit value into the senseLimit value
    val = map(val, 1, senseLimit, 1, 1023);  // remap the constrained value within a 1 to 1023 range

    total -= readings[index];               // subtract the last reading
    readings[index] = val; // read from the sensor
    total += readings[index];               // add the reading to the total
    index = (index + 1);                    // advance to the next index

    if (index >= NUMREADINGS)               // if we're at the end of the array...
      index = 0;                            // ...wrap around to the beginning

    average = total / NUMREADINGS;          // calculate the average
 
    //percentile = average/NUMREADINGS;

    if (average > 50){                // if the average is over 50 ...
      showLED0();   // Show a 0

/*
      for (int j=0;j<2;j++){          // arduino led se pali i gasi 3 puta
        digitalWrite(led0,HIGH);
        delay(1000);
        digitalWrite(led0,LOW);
        delay(500);
      }
    */


    }

    if (average > 150){               // and so on ...
      showLED1();   // Show a 1
    }

    if (average > 250){
      //showLED2();   // Show a 2
    }

    if (average > 350){
      showLED3();   // Show a 3
    }


    if (average > 450){
      //showLED4();   // Show a 4
      showLED2();
    }

    if (average > 550){
      showLED5();   // Show a 5
    }

    if (average > 650){
      showLED6();   // Show a 6

    }

    if (average > 750){
      showLED7();   // Show a 7
    }

    if (average > 850){
      showLED8();   // Show a 8
    }

    if (average > 950){
      showLED9();   // Show a 9
    }
    Serial.println(average); // use output to aid in calibrating
    //Serial.println(percentile);


    delay(updateTime);
  }

}
//Show the number 0
void showLED0(){
  LEDlow();
  digitalWrite(LED1, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
  digitalWrite(LED7, HIGH); 
}
//Show the number 1
void showLED1(){
  LEDlow();
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
}
//Show the number 2
void showLED2(){
  LEDlow();
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED6, HIGH);
  digitalWrite(LED7, HIGH); 
}
//Show the number 3
void showLED3(){
  LEDlow();
  digitalWrite(LED2, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
  digitalWrite(LED7, HIGH); 
}
//Show the number 4
void showLED4(){
  LEDlow();
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
}
//Show the number 5
void showLED5(){
  LEDlow();
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED7, HIGH); 
}
//Show the number 6
void showLED6(){
  LEDlow();
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED7, HIGH); 
}
//Show the number 7
void showLED7(){
  LEDlow();
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
  digitalWrite(LED7, HIGH); 
}
//Show the number 8
void showLED8(){
  LEDlow();
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
  digitalWrite(LED7, HIGH); 
}
//Show the number 9
void showLED9(){
  LEDlow();
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
  digitalWrite(LED7, HIGH); 
}
//Show the letter e
void showLEDe(){
  LEDlow();
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED7, HIGH);
}
//Show the letter f
void showLEDf(){
  LEDlow();
  digitalWrite(LED1, HIGH);
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED7, HIGH); 
}
//Show the letter d
void showLEDd(){
  LEDlow();
  digitalWrite(LED2, HIGH);
  digitalWrite(LED3, HIGH);
  digitalWrite(LED4, HIGH);
  digitalWrite(LED5, HIGH);
  digitalWrite(LED6, HIGH);
}
//Resets the display
void LEDlow(){
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
  digitalWrite(LED5, LOW);
  digitalWrite(LED6, LOW);
  digitalWrite(LED7, LOW); 
}    

void intro(){
  //The nice little led intro 'EFD0123456789'
  showLEDe();
  delay(1000);
  showLEDf();
  delay(1000);
  showLEDd();
  delay(1000);
  showLED0();
  delay(300);
  showLED1();
  delay(300);
  showLED2();
  delay(300);
  showLED3();
  delay(300);
  showLED4();
  delay(300);
  showLED5();
  delay(300);
  showLED6();
  delay(300);
  showLED7();
  delay(300);
  showLED8();
  delay(300);
  showLED9();
  delay(300);
  LEDlow();

}