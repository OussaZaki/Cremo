const int analogPin[16] = { A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15 };

const int startingLed = 30;
const int endingLed = 45;

const int startingWixel = 3;
const int endingWixel = 6;

const int bouttonPoussoir = 2;

int analogPointer;
const int classes[] = {510, 404, 767, 904};
int offset = 2;

void setup() {
  // put your setup code here, to run once:
  pinMode(bouttonPoussoir, INPUT);
  
  for (int thisPin = startingLed; thisPin <= endingLed; thisPin++) {
    pinMode(thisPin, OUTPUT);
    digitalWrite(thisPin, HIGH);
  }
  
  for (int thisPin = startingWixel; thisPin <= endingWixel; thisPin++) {
    pinMode(thisPin, OUTPUT);
    digitalWrite(thisPin, LOW);
  }
  
  Serial.begin(9600);
}


void avancer(){
                digitalWrite(3, HIGH);
                digitalWrite(4, LOW);
                digitalWrite(5, LOW);
                delay(900);
}

void tournerDroite(){
                digitalWrite(4, HIGH);
                digitalWrite(3, LOW);
                digitalWrite(5, LOW);
                delay(500);
}

void tournerGauche(){
                digitalWrite(5, HIGH);
                digitalWrite(4, LOW);
                digitalWrite(3, LOW);
                delay(500);
}

void loop() {
  digitalWrite(3, LOW);
  digitalWrite(4, LOW);
  digitalWrite(5, LOW);
  if (Serial.available() > 0) 
  {
          int thisChar = Serial.read();
          if(thisChar == 'a')
          {
                avancer();
          }
          if(thisChar == 'd')
          {
                tournerDroite();
          }
          if(thisChar == 'g')
          {
                tournerGauche();
          }
  }
}
