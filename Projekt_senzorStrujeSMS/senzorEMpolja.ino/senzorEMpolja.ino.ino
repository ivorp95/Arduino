#define NUMREADINGS 32        // raise this number to increase data smoothing

int senseLimit = 12; // raise this number to decrease sensitivity (up to 1023 max)
int probePin = 5; // analog 5
int val = 0; // reading from probePin
int trueVal=0, trueValConstrain=0;
//float EMA_a = 0.06;

int LED0=13;    //interlnal led
int LED2 = 3;
int LED3 = 4;
int LED4 = 5; 


// variables for smoothing

int readings[NUMREADINGS];                // the readings from the analog input
int index = 0;                            // the index of the current reading
int total = 0;                            // the running total
int average = 0;                          // final average of the probe reading

//CHANGE THIS TO affect the speed of the updates for numbers. Lower the number the faster it updates.
int updateTime = 60;

void setup() {
  pinMode(3, OUTPUT); 
  pinMode(4, OUTPUT); 
  pinMode(5, OUTPUT); 
  pinMode(13, OUTPUT); 

  Serial.begin(38400);

  for (int i = 0; i < NUMREADINGS; i++)
    readings[i] = 0;          
    intro();
}





void loop() {
  LEDlow();
  val = analogRead(probePin);  // take a reading from the probe
  trueVal=val;

  if(val > 0){                // if the reading isn't zero, proceed
    val = constrain(val, 1, senseLimit);  // turn any reading higher than the senseLimit value into the senseLimit value
    val = map(val, 1, senseLimit, 1, 1023);  // remap the constrained value within a 1 to 1023 range
    trueValConstrain=val;

    total -= readings[index];               // subtract the last reading
    readings[index] = val; // read from the sensor
    total += readings[index];               // add the reading to the total
    index = (index + 1);                    // advance to the next index

    if (index >= NUMREADINGS)               // if we're at the end of the array...
      index = 0;                            // ...wrap around to the beginning

    average = total / NUMREADINGS;          // calculate the average

    if (average > 50){
      showLED2();
    }

    if (average > 350){
      showLED4();
    }

    if (average > 850){
      showLED3();
    }

    Serial.println(average); // use output to aid in calibrating

    delay(updateTime);
  }

}






void showLED2(){
  LEDlow();
  digitalWrite(LED2, HIGH);
}

void showLED3(){
  LEDlow();
  digitalWrite(LED3, HIGH);
}

void showLED4(){
  LEDlow();
  digitalWrite(LED4, HIGH);
}

void LEDlow(){
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(LED4, LOW);
}    

void intro(){
  showLED2();
  delay(300);
  LEDlow();
  showLED3();
  delay(300);
  LEDlow();
  showLED4();
  delay(300);
  LEDlow();
}