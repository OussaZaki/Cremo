const int analogPin[16] = { A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15 };

const int startingLed = 30;
const int endingLed = 45;

const int startingWixel = 3;
const int endingWixel = 6;

const int bouttonPoussoir = 2;

int analogPointer;
//const int classes[] = {510, 404, 767, 904};
const int classes[] = {510, 404, 833, 925};
const int classTolerence = 5;

const int dureeAvancement = 800;
const int dureeRotation = 700;
const int dureeArret = 600;

void avancer(){
                digitalWrite(3, HIGH);
                digitalWrite(4, LOW);
                digitalWrite(5, LOW);
                delay(dureeAvancement);
}

void tournerDroite(){
                digitalWrite(4, HIGH);
                digitalWrite(3, LOW);
                digitalWrite(5, LOW);
                delay(dureeRotation);
}

void tournerGauche(){
                digitalWrite(5, HIGH);
                digitalWrite(4, LOW);
                digitalWrite(3, LOW);
                delay(dureeRotation);
}
void stoop(){
                digitalWrite(3, LOW);
                digitalWrite(4, LOW);
                digitalWrite(5, LOW);
                delay(dureeArret);
}

void setup() {
  // put your setup code here, to run once:
  pinMode(bouttonPoussoir, INPUT);
  
  for (int thisPin = startingLed; thisPin <= endingLed; thisPin++) {
    pinMode(thisPin, OUTPUT);
    digitalWrite(thisPin, HIGH);
  }
  
  for (int thisPin = startingWixel; thisPin <= endingWixel; thisPin++) {
    pinMode(thisPin, OUTPUT);
  }
  
  Serial.begin(9600);
}

void loop() {
              Serial.println("start reading");
              for (int thisPin = 0; thisPin <= 15; thisPin++) 
              {
                  Serial.print("pin A");
                  Serial.print(thisPin);
                  Serial.print(": ");
                  analogPointer = analogRead(thisPin);
                  Serial.println(analogPointer);
                  if(analogPointer < 1000)
                  {
                      for (int thisClass = 0; thisClass < 4; thisClass++)
                      {
                            if(analogPointer > (classes[thisClass] - classTolerence) && analogPointer < (classes[thisClass] + classTolerence))
                            {
                                Serial.print("classe");
                                Serial.println(thisClass + 1);
                                digitalWrite(thisPin + startingLed, LOW);
                                break;
                            }   
                      }
                  }
                  else{
                    digitalWrite(thisPin + startingLed, HIGH);
                  }
              }
              stoop();
}
