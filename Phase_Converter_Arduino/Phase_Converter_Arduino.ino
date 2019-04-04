/*******************************************************************
 * Self-Starting Rotary Phase Converter
 * 
 * Client - Glen Phoenix Phase Converters
 *  
 *  This is a device that will automatically start up when
 *  AC current is detected. Used to time the power sequence
 *  for industrial equipment.
 *
 * Last Revision by Eric Cho on March 2018: 
 * https://www.github.com/ericchos/RPI_Proj_1620
*******************************************************************/

// Settings
//Defines whether debug output will be used. Comment out if not using 
#define DEBUG 
#ifdef DEBUG 
  #define D(x) Serial.print(x)
#else
  #define D(x)
#endif

#include <PZEM004T.h>
#include <SoftwareSerial.h>
#include "EmonLib.h"  // Include Emon Library
EnergyMonitor idler;  // SMAKN Voltage Transformer Module 
PZEM004T pzem(12,11);  // (RX,TX) connect to TX,RX of PZEM
IPAddress ip(192,168,1,1);

// Arduino Pin Definitions
const byte IDLER_VTG = A0;          //Analog Input To Pin A0
const byte RPIGPIO_20_START = A1;   //From Rapsberry PI GPIO20
const byte RPIGPIO_21_FINISH = A2;  //From Rapsberry PI GPIO21
const byte UNUSED_RELAY = 6;        //To Pin D6 
const byte LOAD_RELAY = 3;          //To Pin D3
const byte IDLER_RELAY = 4;         //To Pin D5
const byte STARTCAP_RELAY = 5;      //To Pin D4
const byte IDLER_STATE = 10;        //To Pin D10
const byte LED = 13;

// Set the Voltage Limits
const int IDLER_ON_LIMIT = 0; //VAC
const int IDLER_OFF_LIMIT = 0; //VAC
#define STARTCAP_TIME 3000UL
#define IDLER_RUNTIME 180000UL

// Global Variables
int lastPowerStatus = true;
int PowerStatus = true; // Indicates if the power is on or off

// Set Up Routine.Runs first in the Program
void setup() {
  //Testing Mode. Comment out when not in use
  //testmode(); 
  PowerStatus = true;

  // Start Arduino to PC Serial Comm
  Serial.begin(115200);
  
  // Set Arduino GPIO pins to Output
  pinMode(RPIGPIO_20_START, INPUT);
  pinMode(RPIGPIO_21_FINISH, INPUT);
  pinMode(LOAD_RELAY, OUTPUT);
  pinMode(STARTCAP_RELAY, OUTPUT);
  pinMode(IDLER_RELAY, OUTPUT);
  pinMode(UNUSED_RELAY, OUTPUT);
  pinMode(IDLER_STATE, INPUT);
  pinMode(LED, OUTPUT);
  
  // Set Idler Voltage Sensor Parameters
  idler.voltage(IDLER_VTG, 248, 1.7);
  
  // Set Default States for the Relays
  digitalWrite(LOAD_RELAY, HIGH);   // EXTERNAL EQUIPMENT. NORMALLY CLOSED
  digitalWrite(STARTCAP_RELAY, LOW);  // START CAPACITORS
  digitalWrite(IDLER_RELAY, LOW);  // IDLER MOTOR
  digitalWrite(UNUSED_RELAY, LOW);  // Unused Relay
  /*
  while(1){
    float v = pzem.voltage(ip);
    float i = pzem.current(ip);
    Serial.print(v);
    Serial.print(" VAC ");
    Serial.print(i);
    Serial.println(" ~A");
  }
  */
}

#define SETPOINT 1.0
// Main Loop. This repeats over the whole operation.
void loop() {
  //float v = pzem.voltage(ip);
  float i = pzem.current(ip);
  
  // 1: Check if Raspberry Pi has sent start signal from detecting
  //    current from the equipment being powered on
  if(i >= SETPOINT){
    digitalWrite(LED, HIGH);
    Serial.println("Step 1: Shut down load. Start Capaciter & Idler");
    // Turn off the load relay and switch on the start caps
    digitalWrite(LOAD_RELAY, LOW);
    digitalWrite(IDLER_RELAY, HIGH);
    delay(10);
    
    // 2:If voltage is detected on the idler motor, then
    //   turn off the start capcacitors and turn on the load
    //   relay to run off the load power source
    Serial.println("Step 2: Waiting for Idler Voltage to Rise");
    while(digitalRead(IDLER_STATE) != true)
      delay(10);
    delay(500);
    Serial.println("Step 4: Ider Voltage has gone up. Turning off Start Caps & switching back to load");
    digitalWrite(LOAD_RELAY, HIGH);
    
    // 4: If the Raspberry Pi detects low current due to the 
    //    user shutting off the equipment, the load circuit 
    //    will be disconnected and the idler motor will continue to run.
    
    do{
      i = pzem.current(ip);
      Serial.println(i);
      delay(10);
    }while(i > 1.0);
    
    // Shut down sequence
    Serial.println("Step 5: Shut Down Sequence.");
    digitalWrite(LOAD_RELAY, HIGH);
    digitalWrite(IDLER_RELAY, LOW);
    delay(100);
    digitalWrite(LED, LOW);
  }
}

float analogAverage(int pin){
  float average = 0;
  for(int i = 0; i<300; i++){
    average += analogRead(pin);
  }
  return average = (float)average/300.0;
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
  pinMode(13, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(3, OUTPUT);
  
  //Set the Initial Relay States
  byte test = HIGH;
  byte relay1 = HIGH;
  byte relay2 = LOW;
  byte relay3 = LOW;
  byte relay4 = LOW;
  digitalWrite(13, test);
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
      case '5':
        test = !test;
        digitalWrite(13, test);
        Serial.print("LED 13: ");
        Serial.println(test);
        break;
      case '6':
        Serial.print(analogRead(A0));
        break;
    }
  }
}
