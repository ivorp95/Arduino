
int Crvena = 16
int Zelena = 18;
int Plava = 17;


void setup()
{
  Serial.begin(9600);
  
  pinMode(7, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);

  Serial.println("Unos naziva boje i stanja LED diode -crvena/zelena/plava 1/0-");
  Serial.println("(crvena 1 - pali crvenu LED, crvena 0 - gasi crvenu LED)");
}

void loop()
{
  String unos;

  while(Serial.available()){

    unos=Serial.readString();
    //Serial.println(unos);

    if (unos=="Crvena 1" || unos=="crvena 1" || unos=="CRVENA 1"){
        digitalWrite(Crvena, HIGH);
        Serial.println("Crvena LED upaljena");
    }
    else if(unos=="Crvena 0" || unos=="crvena 0" || unos=="CRVENA 0"){
        digitalWrite(Crvena, LOW);
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