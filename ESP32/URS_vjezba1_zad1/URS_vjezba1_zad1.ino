
int Crvena = 16;
int Zelena = 18;
int Plava = 17;

int internalLED=13;

char *stringovi[]{"Crvena 1","Crvena 0", "crvena 1", "crvena 0", "CRVENA 1","CRVENA 0",
"Zelena 1","Zelena 0","zelena 1","zelena 0","ZELENA 1","ZELENA 0",
"Plava 1","Plava 0","plava 1","plava 0","PLAVA 1","PLAVA 0"};

String unos;


void setup()
{
  Serial.begin(9600);
  
  pinMode(Crvena, OUTPUT);
  pinMode(Zelena, OUTPUT);
  pinMode(Plava, OUTPUT);

  pinMode(internalLED, OUTPUT);

  Serial.println("Unos naziva boje i stanja LED diode -crvena/zelena/plava 1/0-");
  Serial.println("(crvena 1 - pali crvenu LED, crvena 0 - gasi crvenu LED)");
}

void loop()
{


  while(Serial.available()){

    //digitalWrite(13, HIGH);
    //Serial.println("testis 2");

    unos=Serial.readString();
    Serial.println(unos);

    if (unos==stringovi[0] || unos==stringovi[2] || unos==stringovi[4]){
        digitalWrite(Crvena, HIGH);
        digitalWrite(internalLED, HIGH);
        Serial.println("Crvena LED upaljena");
    }
    else if(unos==stringovi[1] || unos==stringovi[3] || unos==stringovi[5]){
        digitalWrite(Crvena, LOW);
        digitalWrite(internalLED, LOW);
        Serial.println("Crvena LED ugasena");
    }   


    else if(unos=="Zelena 1" || unos=="zelena 1"|| unos=="ZELENA 1"){
        digitalWrite(Zelena, HIGH);
        Serial.println("Zelena LED upaljena");
    }
    else if(unos=="Zelena 0" || unos=="zelena 0"|| unos=="ZELENA 0"){
        digitalWrite(Zelena, LOW);
        Serial.println("Zelena LED ugasena");
    }


    else if(unos=="Plava 1" || unos=="plava 1" || unos=="PLAVA 1"){
        digitalWrite(Plava, HIGH);
        Serial.println("Plava LED upaljena");
    }
    else if(unos=="Plava 0" || unos=="plava 0" || unos=="PLAVA 0"){
        digitalWrite(Plava, LOW);
        Serial.println("Plava LED ugasena");
    }
  
  }


}