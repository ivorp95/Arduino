
#define GRANICA 305

//analog
int analogSensor=1;

//digital
int LEDcrvena=5;
int LEDzelenaPWM=9;

//varijable
float napon=0;
int ocitanje=0;
int svijetlost=0;

void setup() {
  pinMode(LEDcrvena, OUTPUT);
  pinMode(LEDzelenaPWM, OUTPUT);

  Serial.begin(9600);

}


void loop() {

  
  svijetlost=analogRead(analogSensor);
  Serial.println(svijetlost);

  if(svijetlost<GRANICA){
  	digitalWrite(LEDcrvena, 1);
  }

    	if(svijetlost>GRANICA){
     	 ocitanje=analogRead(analogSensor);
    	  napon=ocitanje*(5.0/1023.0);
    	  Serial.println(napon);

    	  if (napon>2.0){
    	    digitalWrite(LEDcrvena, 0);

     	   for (int i=127; i<256; i+=5){
      	    analogWrite(LEDzelenaPWM, i);
    	     delay(3);
        	}
     	   for (int i=256; i>127; i-=5){
      	    analogWrite(LEDzelenaPWM, i);
      	    delay(3);
        	}
      	}
       }
    else {
      digitalWrite(LEDcrvena, 1);
      digitalWrite(LEDzelenaPWM, 0);
    }


}