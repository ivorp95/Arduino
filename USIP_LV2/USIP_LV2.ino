
#define VRIJEME 2000

//analog
int analogSensor=1;

//digital
int tipkalo=2;
int LEDcrvena=3;
int LEDzelenaPWM=9;

//flagovi, 1 zbog INPUT_PULLUP
int stanjeTipkala=1;
int zadnjeStanjeTipkala=1;

//varijable
float napon=0;
int ocitanje=0;
 

void setup() {
  pinMode(tipkalo, INPUT_PULLUP);
  pinMode(LEDcrvena, OUTPUT);
  pinMode(LEDzelenaPWM, OUTPUT);

  Serial.begin(9600);

}

void loop() {

  unsigned long trajanjeTipkala=0;

  stanjeTipkala=digitalRead(tipkalo);
  Serial.println(stanjeTipkala);

  digitalWrite(LEDcrvena, 1);

  if (stanjeTipkala==0 && zadnjeStanjeTipkala==1){
    trajanjeTipkala=millis();
  }
    if(stanjeTipkala==0 && trajanjeTipkal>VRIJEME){
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
