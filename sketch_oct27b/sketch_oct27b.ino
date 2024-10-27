
int ledTipkalo = 3; 


void setup() {
  // put your setup code here, to run once:
  pinMode(3,OUTPUT);


}

void loop() {
  // put your main code here, to run repeatedly:
  //digitalWrite(ledTipkalo,HIGH);
  ledSOS();


}

void ledSOS(){
  digitalWrite(ledTipkalo,HIGH);      // S
  delay(500);
  digitalWrite(ledTipkalo,LOW);
  delay(500);
  digitalWrite(ledTipkalo,HIGH);
  delay(500);
  digitalWrite(ledTipkalo,LOW);
  delay(500);
  digitalWrite(ledTipkalo,HIGH);
  delay(500);
  digitalWrite(ledTipkalo,LOW);
  delay(500);

  digitalWrite(ledTipkalo,HIGH);        // O
  delay(1500);
  digitalWrite(ledTipkalo,LOW);
  delay(500);
  digitalWrite(ledTipkalo,HIGH);
  delay(1500);
  digitalWrite(ledTipkalo,LOW);
  delay(500);
  digitalWrite(ledTipkalo,HIGH);
  delay(1500);
  digitalWrite(ledTipkalo,LOW);
  delay(500);

  digitalWrite(ledTipkalo,HIGH);        // S
  delay(500);
  digitalWrite(ledTipkalo,LOW);
  delay(500);
  digitalWrite(ledTipkalo,HIGH);
  delay(500);
  digitalWrite(ledTipkalo,LOW);
  delay(500);
  digitalWrite(ledTipkalo,HIGH);
  delay(500);
  digitalWrite(ledTipkalo,LOW);
  delay(5000);

}
