const int analogPin[16] = { A0, A1, A2, A3, A4, A5, A6, A7,
                           A11, A10, A9, A8, A12, A13, A15, A14 };
                           
const int digitalPin[16] = { 30, 31, 32, 33, 37, 36, 35, 34,
                           38, 39, 40, 41, 44, 45, 42, 43 };

const int startingWixel = 9;
const int endingWixel = 11;

const int bouttonPoussoir = 22;

int c;
int analogPointer;
int instructions[16];
const int classes[] = { 904 ,510, 400, 767};
const int classTolerence = 10;

const int dureeAvancement = 800;
const int dureeRotation = 600;
const int dureeArret = 600;

void avancer(){
    digitalWrite(11, HIGH);
    digitalWrite(10, LOW);
    digitalWrite(9, LOW);
    delay(dureeAvancement);
}

void tournerDroite(){
    digitalWrite(10, HIGH);
    digitalWrite(11, LOW);
    digitalWrite(9, LOW);
    delay(dureeRotation);
}

void tournerGauche(){
    digitalWrite(9, HIGH);
    digitalWrite(10, LOW);
    digitalWrite(10, LOW);
    delay(dureeRotation);
}
void stoop(){
    digitalWrite(9, LOW);
    digitalWrite(10, LOW);
    digitalWrite(11, LOW);
    delay(dureeArret);
}

void setup() {
    pinMode(bouttonPoussoir, INPUT);
  
    for (int thisPin = 0; thisPin <= 15; thisPin++) {
        pinMode(digitalPin[thisPin], OUTPUT);
        digitalWrite(digitalPin[thisPin], HIGH);
    }
  
    for (int thisPin = startingWixel; thisPin <= endingWixel; thisPin++) {
        pinMode(thisPin, OUTPUT);
    }
  
    Serial.begin(9600);   
}
  
void loop() {

    for(int i=0;i<16;i++)
    {
        instructions[i]=-1;
    }
    for (int thisPin = 0; thisPin <= 15; thisPin++) 
    {
        analogPointer = analogRead(analogPin[thisPin]);
        if(analogPointer < 1000)
        {
            for (int thisClass = 0; thisClass < 4; thisClass++)
            {
                if(analogPointer > (classes[thisClass] - classTolerence) && analogPointer < (classes[thisClass] + classTolerence))
                {
                    instructions[thisPin] = thisClass;
                    digitalWrite(digitalPin[thisPin], LOW);
                    break;
                }   
            }
        }
        else
        {
            digitalWrite(digitalPin[thisPin], HIGH);
        }
    }
    stoop();
    c = digitalRead(bouttonPoussoir);
      if(c == 1)
      {
        for (int thisPin = 0; thisPin <= 15; thisPin++) {
          digitalWrite(digitalPin[thisPin], HIGH);
        }
        delay(2000);
        for(int i=0;i<12;i++)
        {
            switch (instructions[i])
            {
                case 0:
                    digitalWrite(digitalPin[i], LOW);
                    for(int j=12;j<16;j++)
                    {
                        switch (instructions[j])
                        {
                            case 1:
                                digitalWrite(digitalPin[j], LOW);
                                avancer();
                                stoop();
                                break; 
                            case 2:
                                digitalWrite(digitalPin[j], LOW);
                                tournerGauche();
                                stoop();
                                break; 
                            case 3:
                                digitalWrite(digitalPin[j], LOW);
                                tournerDroite();
                                stoop();
                            break; 
                            default:
                            break; 
                        }
                    }
                    break; 
                case 1:
                    digitalWrite(digitalPin[i], LOW);
                    avancer();
                    stoop();
                    break; 
                case 2:
                
                    digitalWrite(digitalPin[i], LOW);
                    tournerGauche();
                    stoop();
                    break; 
                case 3:
                    digitalWrite(digitalPin[i], LOW);
                    tournerDroite();
                    stoop();
                    break;
                default:
                     break; 
            }
        }
    }
}      
