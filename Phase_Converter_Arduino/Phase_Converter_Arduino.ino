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

// Arduino Pin Definitions
const byte IDLER_VTG = A0;     //Analog Input To Pin A0
const byte RPIGPIO_20 = A1;    //From Rapsberry PI GPIO20
const byte RPIGPIO_21 = A2;    //From Rapsberry PI GPIO21
const byte LOAD_RELAY = 6;     //To Pin D6 
const byte STARTCAP_RELAY = 5; //To Pin D5
const byte IDLER_RELAY = 4;    //To Pin D4
const byte RELAY_4 = 3;        //To Pin D3

// Global Variables
int PowerStatus = 0;

// Set Up Routine.Runs first in the Program
void setup() {
  //Testing Mode. Comment out when not in use
  //testmode(); 
  
  PowerStatus = 0;
  
  // Start Arduino to PC Serial Comm
  Serial.begin(9600); 
  
  // Set Arduino GPIO pins to Output
  pinMode(RPIGPIO_20, INPUT);
  pinMode(RPIGPIO_21, INPUT);
  pinMode(LOAD_RELAY, OUTPUT);
  pinMode(STARTCAP_RELAY, OUTPUT);
  pinMode(IDLER_RELAY, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  
  // Set Idler Voltage Sensor Parameters
  idler.voltage(IDLER_VTG, 248, 1.7);
  
  // Set Default States for the Relays
  digitalWrite(LOAD_RELAY, LOW);   // EXTERNAL EQUIPMENT
  digitalWrite(STARTCAP_RELAY, HIGH);  // START CAPACITORS
  digitalWrite(IDLER_RELAY, HIGH);  // IDLER MOTOR
  digitalWrite(RELAY_4, HIGH);  // 
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
  
  /*
  if (idlerVoltage > 6 && PowerStatus == 0) {
      PowerStatus = 1;
      StartConverter();
  }
  else if (idlerVoltage < 220 && PowerStatus == 1) {
    StopConverter();
  }
  */
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
  pinMode(LOAD_RELAY, OUTPUT);
  pinMode(STARTCAP_RELAY, OUTPUT);
  pinMode(IDLER_RELAY, OUTPUT);
  pinMode(RELAY_4, OUTPUT);
  
  //Set the Initial Relay States
  byte relay1 = 1;
  byte relay2 = 0;
  byte relay3 = 0;
  byte relay4 = 0;
  digitalWrite(LOAD_RELAY, relay1);
  digitalWrite(STARTCAP_RELAY, relay2);
  digitalWrite(IDLER_RELAY, relay3);
  digitalWrite(RELAY_4, relay4);
  
  while(1){
    // Receive character from Bluetooth Module 
    char rxData = Serial.read();
    
    switch(rxData){
      case '1':
        relay1 = !relay1;
        digitalWrite(LOAD_RELAY, relay1);
        Serial.print("Switching Load Relay: ");
        Serial.println(relay1);
        break;
      case '2':
        relay2 = !relay2;
        digitalWrite(STARTCAP_RELAY, relay2);
        Serial.print("Switching Start Capacitor Relay: ");
        Serial.println(relay2);
        break;
      case '3':
        relay3 = !relay3;
        digitalWrite(IDLER_RELAY, relay3);
        Serial.print("Switching IDLER Relay: ");
        Serial.println(relay3);
        break;
      case '4':
        relay4 = !relay4;
        digitalWrite(RELAY_4, relay4);
        Serial.print("Switching Relay 4: ");
        Serial.println(relay4);
        break;
    }
  }
}
