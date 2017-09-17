#include <LiquidCrystal.h>

#define DELAY_VACUUM_OFF 6000
#define DELAY_AUTOPOWER_OFF 1800000 //On coupe au bout de 30 minutes

int sensorTA12 = A0; // Analog input pin that sensor is attached to
int relay1 = 6; // Command relay 1
int relay2 = 7; // Command relay 2
int relay3 = 8; // Command relay 3
int relay4 = 9; // Command relay 4
int etat_relais1 = 0;

int retroled = 10;

float nVPP;   // Voltage measured across resistor
float nCurrThruResistorPP; // Peak Current Measured Through Resistor
float nCurrThruResistorRMS; // RMS current through Resistor
float nCurrentThruWire;     // Actual RMS current in Wire

const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

uint32_t tempo_off,tempo_on ;
uint32_t tempo_retro ;       //timestamp pour savoir quand couper le retro eclairage


void setup() {
   //Serial.begin(9600); 
   pinMode(sensorTA12, INPUT);
   
   pinMode(relay1,OUTPUT);digitalWrite(relay1,HIGH);
   pinMode(relay2,OUTPUT);digitalWrite(relay2,HIGH);
   pinMode(relay3,OUTPUT);digitalWrite(relay3,HIGH);
   pinMode(relay4,OUTPUT);digitalWrite(relay4,HIGH);

   pinMode(retroled,OUTPUT);digitalWrite(retroled,HIGH);
   
   //pinMode(6,OUTPUT);pinMode(8,OUTPUT);
   etat_relais1=0;
     
   lcd.begin(16, 2);
   tempo_on = 0;
   tempo_off = 0;

   tempo_retro=0;

   lcd.setCursor(0, 0);
   lcd.print("Machine: OFF");
   lcd.setCursor(0, 1);
   lcd.print("Aspirateur: OFF");
}

void loop() {
   nVPP = getVPP(); //nVPP=0;
   Serial.print("Volts Peak : "); Serial.println(nVPP,3);


   if(nVPP>0.150){ //doit être allumé
     if(etat_relais1==0) { //Il est eteint 
       if(tempo_on<(millis()-1000)){ //Retard à l'allumage  
         etat_relais1=1;
         tempo_off=millis(); //je repousse la tempo d'extinction de l'aspi
         digitalWrite(relay1,LOW); //On allume l'aspi
         //tempo_retro=millis(); //Changement d'état alors on active le retro eclairage
         //Serial.println("Relais ON");Serial.println("");Serial.println("");Serial.println("");
         lcd.setCursor(0, 1);
         lcd.print("Aspirateur: ON ");
       }
     } else {//déjà allumé
       tempo_off=millis(); //je repousse la tempo d'extinction
       lcd.setCursor(0, 1);
       lcd.print("Aspirateur: ON ");
     }
     tempo_retro=millis(); //Est allumé ou va s'allumer.
   } else { //doit être éteint
     if (etat_relais1==1){ //Il est allumé
       if (tempo_off<(millis()-DELAY_VACUUM_OFF)){ //Retard à l'extinction
         etat_relais1=0;
         digitalWrite(relay1,HIGH); //On éteind l'aspi  
         //Serial.println("Relais OFF");
         lcd.setCursor(0, 1);
         lcd.print("Aspirateur: OFF");
       } else {
         lcd.setCursor(0, 1);
         lcd.print("Aspirateur: ");
         lcd.setCursor(13, 1);
         lcd.print(1+(tempo_off-(millis()-DELAY_VACUUM_OFF))/1000);
         lcd.print("    ");
       }
       //tempo_retro=millis(); //Changement d'état prévu alors on active le retro eclairage
     } else {//déjà éteint
       tempo_on=millis(); //je repousse la tempo d'allumage
     }
   }
 
   lcd.setCursor(0, 0);
   if(nVPP>0.150)     
     lcd.print("Machine: ON    ");
   else {
     lcd.print("Machine: OFF /");
     int aso = (DELAY_AUTOPOWER_OFF-(millis()-tempo_retro))/60000;
     lcd.print(aso);
     Serial.print("Machine: OFF /");
     Serial.println(aso);
   }

   //Retro éclairage de l'écran
   if (millis()<15000 || tempo_retro>(millis()-15000))
     digitalWrite(retroled,HIGH);
   else
     digitalWrite(retroled,LOW);

   if (millis()-tempo_retro>(DELAY_AUTOPOWER_OFF)){ //Dernière activité y'a 15 minutes je coupe tout
     digitalWrite(retroled,HIGH);
     lcd.setCursor(0, 0);
     lcd.print(" ARRET          ");
     lcd.setCursor(0, 1);
     lcd.print("    AUTOMATIQUE ");
     delay(2000);
     
     digitalWrite(relay4,LOW); //ON
     delay(300);
     digitalWrite(relay4,HIGH); //OFF
     tempo_retro+=20000; //JE NE DOIS JAMAIS TOMBER DEDANS PUISQUE LE TELERUPTEUR AURA SWITCHé   
   }
   


   
   /*Serial.print("etat relay : ");
   Serial.println(etat_relais1);
   Serial.print("tempo_on : ");
   Serial.println(tempo_on);*/
   delay(100);
}


float getVPP()
{
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
   uint32_t start_time = millis();
   while((millis()-start_time) < 100) //sample for 0.1 Sec
   {
       readValue = analogRead(sensorTA12);
       // see if you have a new maxValue
       if (readValue > maxValue) 
       {
           /*record the maximum sensor value*/
           maxValue = readValue;
       }
   }
   
   // Convert the digital data to a voltage
   result = (maxValue * 5.0)/1024.0;
  
   return result;
 }
