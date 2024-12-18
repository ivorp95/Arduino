
int Crvena = 13;
int Zelena = 18;
int Plava = 17;

int trajanje = 200; //trajanje tockice 200ms, crtica je 3xtockica 


char *morse_slova[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..",    
  ".---", "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.",  
  "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--.."         
};
char *morse_brojevi[] = {
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};

void ispis_morse_kod (char *slovoIliBroj);
void ispis_tocka_crta (char tockaIliCrta);

void setup(){

  Serial.begin(9600);
  
  pinMode(Crvena, OUTPUT);
  pinMode(Zelena, OUTPUT);
  pinMode(Plava, OUTPUT);

  Serial.println("Za uneseni string na Serial konzolu, ispisuje morsov kod na LED diodi");
}


void loop(){

  char unos;


  if (Serial.available()){
    unos=Serial.read();

    if (unos >= 'A' && unos <= 'Z'){
      Serial.println(unos);
      ispis_morse_kod(morse_slova[unos-'A']);   //decimalni asci unosa - pomak do prvog slova, daje broj indexa slova ili broja
    }
      else if (unos >= 'a' && unos <= 'z'){
        Serial.println(unos);
        ispis_morse_kod(morse_slova[unos-'a']);
    }
          else if (unos >= '0' && unos <= '9'){
            Serial.println(unos);
            ispis_morse_kod(morse_slova[unos-'0']);
    }
              else if (unos >= ' '){
                delay(trajanje*7);
    }

  }

}

void ispis_morse_kod (char *slovoIliBroj){
  int i=0;

  while (slovoIliBroj[i]!=NULL){
    ispis_tocka_crta(slovoIliBroj[i]);
    i++;
  }
    delay(trajanje*3);

}


void ispis_tocka_crta (char tockaIliCrta){

  digitalWrite(Crvena,HIGH);
  digitalWrite(Zelena,HIGH);
  digitalWrite(Plava,HIGH);

  if (tockaIliCrta=='.'){
    delay(trajanje);
  }
  else {
    delay(trajanje*3);
  }

  digitalWrite(Crvena,LOW);
  digitalWrite(Zelena,LOW);
  digitalWrite(Plava,LOW);
  delay(trajanje);
}

