int PowerStatus = 0;

#include "EmonLib.h"             // Include Emon Library
EnergyMonitor emon1;             // Create an instance
EnergyMonitor emon2;             // Create an instance
EnergyMonitor emon3;             // Create an instance
const byte RELAY_1 = 39;
const byte RELAY_2 = 41;
const byte RELAY_3 = 37;
const byte RELAY_4 = 43;

void setup() {

  PowerStatus = 0;
  Serial.begin(115200);
  pinMode(RELAY_1, OUTPUT);
  pinMode(RELAY_2, OUTPUT);
  pinMode(RELAY_3, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  emon1.voltage(13, 122, 1.7);
  emon2.voltage(14, 122, 1.7);
  emon3.voltage(15, 248, 1.7);
  digitalWrite(RELAY_1, LOW);   // EXTERNAL EQUIPMENT
  digitalWrite(RELAY_2, HIGH);  // START CAPACITORS
  digitalWrite(RELAY_3, HIGH);  // IDLER MOTOR
  digitalWrite(RELAY_4, HIGH);   // 


}

void loop() {

  emon1.calcVI(10, 1024);
  emon2.calcVI(10, 1024);
  emon3.calcVI(10, 1024);
  float L1   = emon1.Vrms;
  float L2   = emon2.Vrms;
  float L3   = emon3.Vrms;
  Serial.print("L1: ");
  Serial.println(L1);
  Serial.print("L2: ");
  Serial.println(L2);
  Serial.print("L3: ");
  Serial.println(L3);
  Serial.print("PowerStatus= ");
  Serial.println (PowerStatus);
  if (L3 > 6) {
    if (PowerStatus == 0) {
      PowerStatus = 1;
      StartConverter();
    }
  }
  else {
    if (L3 < 220) {
      if (PowerStatus == 1) {
        StopConverter();
      }
    }
  }

}

void StartConverter()
{
  float L3 = emon3.Vrms;
 while (L3 < 240)
  {
    digitalWrite(RELAY_1, HIGH);
    digitalWrite(RELAY_2, LOW);
    digitalWrite(RELAY_3, LOW);
    emon3.calcVI(10, 1024);
    L3 = emon3.Vrms;
    Serial.print("STARTING L3: ");
    Serial.println(L3);
  } 
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_1, LOW);
  
}
void StopConverter()
{
  PowerStatus = 0;
  digitalWrite(RELAY_1, LOW);
  digitalWrite(RELAY_2, HIGH);
  digitalWrite(RELAY_3, HIGH);
  digitalWrite(RELAY_4, HIGH);
  delay(15);
}
