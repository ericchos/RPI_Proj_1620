/*******************************************************************
 * Self-Starting Rotary Phase Converter
 *  
 *  This is a device that will automatically start up when
 *  AC current is detected. Used to time the power sequence
 *  for industrial equipment.
 *
 * Last Revision by Eric Cho on March 2018: 
 * https://www.github.com/ericchos/RPI_Proj_1620
*******************************************************************/

#include "EmonLib.h"  // Include Emon Library
EnergyMonitor idler;  // SMAKN Voltage Transformer Module 

// Arduino Pin Definitions#3
const byte IDLER_VTG = A0;     //Analog Input To Pin A0
const byte RPIGPIO_20_START = A1;    //From Rapsberry PI GPIO20
const byte RPIGPIO_21_FINISH = A2;    //From Rapsberry PI GPIO21
const byte UNUSED_RELAY = 6;   //To Pin D6 
const byte IDLER_RELAY = 5;    //To Pin D5
const byte STARTCAP_RELAY = 4; //To Pin D4
const byte LOAD_RELAY = 3;    //To Pin D3

// Set the Voltage Limits
const int IDLER_ON_LIMIT = 0; //VAC
const int IDLER_OFF_LIMIT = 0; //VAC
const int STARTCAP_TIME = 3 * 1000; //ms
const int IDLER_RUNTIME = 3 * 60 * 1000; //ms

// Global Variables
int lastPowerStatus = 0;
int PowerStatus = 0; // Indicates if the power is on or off

// Set Up Routine.Runs first in the Program
void setup() {
  //Testing Mode. Comment out when not in use
  testmode(); 
  
  PowerStatus = 0;
  
  // Start Arduino to PC Serial Comm
  Serial.begin(9600); 
  
  // Set Arduino GPIO pins to Output
  pinMode(RPIGPIO_20_START, INPUT);
  pinMode(RPIGPIO_21_FINISH, INPUT);
  pinMode(LOAD_RELAY, OUTPUT);
  pinMode(STARTCAP_RELAY, OUTPUT);
  pinMode(IDLER_RELAY, OUTPUT);
  pinMode(UNUSED_RELAY, OUTPUT);
  
  // Set Idler Voltage Sensor Parameters
  idler.voltage(IDLER_VTG, 248, 1.7);
  
  // Set Default States for the Relays
  digitalWrite(LOAD_RELAY, HIGH);   // EXTERNAL EQUIPMENT
  digitalWrite(STARTCAP_RELAY, HIGH);  // START CAPACITORS
  digitalWrite(IDLER_RELAY, LOW);  // IDLER MOTOR
  digitalWrite(UNUSED_RELAY, LOW);  // 
}

// Main Loop. This repeats over the whole operation.
void loop() {
  
  // Read Idler Voltage
  idler.calcVI(10, 1024);
  float idlerVoltage   = idler.Vrms;
  
  // Display Idler Voltage Reading on PC Terminal
  Serial.print("Idler Voltage: ");
  Serial.println(idlerVoltage);
  Serial.print("Last Power Status= ");
  Serial.println (PowerStatus);
  
  // 1: Check if Raspberry Pi has sent start signal from detecting
  //    current from the equipment being powered on
  if(digitalRead(RPIGPIO_20_START) == true 
  && digitalRead(RPIGPIO_21_FINISH == false)){
    // Turn off the load relay and switch on the start caps
    PowerStatus = true;
    digitalWrite(LOAD_RELAY, LOW);
    digitalWrite(STARTCAP_RELAY, HIGH);
    digitalWrite(IDLER_RELAY, HIGH);
    delay(STARTCAP_TIME);
  }
  // 2:If voltage is detected on the idler motor, then
  //   turn off the start capcacitors and turn on the load
  //   relay to run off the load power source
  if(idlerVoltage >= 190 
  && PowerStatus == true){
    digitalWrite(STARTCAP_RELAY, LOW);
    digitalWrite(IDLER_RELAY, HIGH);
    digitalWrite(LOAD_RELAY, HIGH);
  }
  // 3: If the Raspberry Pi detects low current due to the 
  //    user shutting off the equipment, the load circuit 
  //    will be disconnected and the idler motor will run for
  //    3 minutes before shutting off.
  if(digitalRead(RPIGPIO_20_START) == true 
    && digitalRead(RPIGPIO_21_FINISH == false)){
      PowerStatus =false;
      digitalWrite(LOAD_RELAY, LOW);
      digitalWrite(STARTCAP_RELAY, LOW);
      
      // Let Idler run for awhile before shutting down
      delay(IDLER_RUNTIME);
      digitalWrite(IDLER_RELAY, LOW);
  }
}

/*
 * This is a Testing mode where you can manually control the
 * relays. Uncomment "testmode()" on void setup() and comment
 * when not in use. 
 *
 * During use, open the Serial Monitor and enter a command to
 * test a relay.
*/
void testmode(){
  Serial.begin(9600);
  
  //Set Relay Pin Direction
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  
  //Set the Initial Relay States
  byte relay1 = 1;
  byte relay2 = 0;
  byte relay3 = 0;
  byte relay4 = 0;
  digitalWrite(6, relay1);
  digitalWrite(5, relay2);
  digitalWrite(4, relay3);
  digitalWrite(3, relay4);
  
  while(1){
    // Receive character from Bluetooth Module 
    char rxData = Serial.read();
    
    switch(rxData){
      case '1':
        relay1 = !relay1;
        digitalWrite(6, relay1);
        Serial.print("Switching Load Relay: ");
        Serial.println(relay1);
        break;
      case '2':
        relay2 = !relay2;
        digitalWrite(5, relay2);
        Serial.print("Switching Start Capacitor Relay: ");
        Serial.println(relay2);
        break;
      case '3':
        relay3 = !relay3;
        digitalWrite(4, relay3);
        Serial.print("Switching IDLER Relay: ");
        Serial.println(relay3);
        break;
      case '4':
        relay4 = !relay4;
        digitalWrite(3, relay4);
        Serial.print("Switching Relay 4: ");
        Serial.println(relay4);
        break;
    }
  }
}

