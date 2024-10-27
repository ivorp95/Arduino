int HallSensor = 10; // Hall sensor is connected to the D10 pin
int hallAnalog = A2;
float hallAnalogVal;
int LED = 13; // onboard LED pin

void setup() {

  pinMode(HallSensor, INPUT); // Hall Effect Sensor pin INPUT
  pinMode(LED, OUTPUT); // LED Pin Output
  pinMode(hallAnalog, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  int sensorStatus = digitalRead(HallSensor); // Check the sensor status
  if (sensorStatus == 1) // Check if the pin high or not
  {
    // if the pin is high turn on the onboard Led
    digitalWrite(LED, HIGH); // LED on

    //testvs
    //Serial.println("Ima struje");
    //delay(1000);
  }
  else  {
    //else turn off the onboard LED
    digitalWrite(LED, LOW);
    //Serial.println("Nema struje");
    //delay(1000);

  };

  //citanje analognih vrijednosti na serial 

  //hallAnalogVal = analogRead(hallAnalog); 
  //Serial.println(hallAnalogVal);


}