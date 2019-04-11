/**********************************************************************************************
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
**********************************************************************************************/
/*
 * Defines whether Serial output will be used. 
 * Comment out if not using 
 */
#define DEBUG 
#ifdef DEBUG 
  #define D(x) Serial.print(x)
#else
  #define D(x)
#endif

/*
 * Uncomment one option for Load Current detection 
 */
//#define PZEM_SENSOR 1
//#define ANALOG_DETECT 1
#define ONOFF_DETECT  1
 
// Include Libraries
#ifdef PZEM_SENSOR
#include <PZEM004T.h>
#include <SoftwareSerial.h>
PZEM004T pzem(12,11);  // (RX,TX) connect to TX,RX of PZEM
IPAddress ip(192,168,1,1);
#endif

// Arduino Pin Definitions
const byte LOAD_STATE = A0;         //Input <- From Current Sensor. Only On/Off signal given to detect current
const byte RPIGPIO_20_START = A1;   //Input <- From Rapsberry PI GPIO20
const byte RPIGPIO_21_FINISH = A2;  //Input <- From Rapsberry PI GPIO21
const byte IDLER_STATE = 10;        //Input <- To Pin D10. A On/Off signal given when Idler starts
const byte LOAD_RELAY = 3;          //Output -> To Pin D3
const byte IDLER_RELAY = 4;         //Output -> To Pin D5
const byte LED = 13;                //Output -> To Pin D13 Arduino Onboard LED
//const byte UNUSED_RELAY = 6;      //UNUSED Output -> To Pin D6
//const byte STARTCAP_RELAY = 5;    //UNUSED Output -> To Pin D4

// Set Constants used for triggering values
const int IDLER_ON_LIMIT = 0; //VAC
const int IDLER_OFF_LIMIT = 0; //VAC
const float SETPOINT = 1.0;

// Global Variables
float loadSensor = 0; // Value of the Load Current used to trigger relay sequence

// Set Up Routine.Runs first in the Program
void setup() {
  //Testing Mode. Comment out when not in use
  //testmode(); 

  // Start Arduino to PC Serial Comm
  Serial.begin(9600);
  D("- Starting Phase Converter Program -\r\n");
  
  // Set Arduino GPIO Input Pins
  pinMode(RPIGPIO_20_START, INPUT);
  pinMode(RPIGPIO_21_FINISH, INPUT);
  pinMode(IDLER_STATE, INPUT);
  pinMode(LOAD_STATE, INPUT);

  // Set Arduino GPIO Output Pins
  pinMode(LOAD_RELAY, OUTPUT);
  pinMode(IDLER_RELAY, OUTPUT);
  pinMode(LED, OUTPUT);
  //pinMode(UNUSED_RELAY, OUTPUT); // Relay D6
  //pinMode(STARTCAP_RELAY, OUTPUT); // Relay D5
  
  // Set Default States for the Relays
  digitalWrite(LOAD_RELAY, HIGH);   // EXTERNAL EQUIPMENT. NORMALLY CLOSED
  digitalWrite(IDLER_RELAY, LOW);  // IDLER MOTOR
  //digitalWrite(STARTCAP_RELAY, LOW);  // START CAPACITORS
  //digitalWrite(UNUSED_RELAY, LOW);  // Unused Relay
}

// Main Loop. This repeats over the whole operation.
void loop() {
/*
 * Step 1: Take Sensor Measurement for the load current
 */
#ifdef CURRENT_DETECT 
  loadSensor = (unsigned int)pzem.current(ip); // Load Current in Amps
#elif ANALOG_DETECT 
  loadSensor = analogRead(A0);
#elif ONOFF_DETECT 
  loadSensor = readAverage(A0, 1000);
#endif

/*
 * Step 2: Check if Load amperage is detected
 */
  if(loadSensor < SETPOINT){
    digitalWrite(LED, HIGH);
    D("Step 1: Shut down load and start idler.\r\n");
    D("\t Load Signal: ");
    D(loadSensor);
    D(" (1 if OFF, Less than 1 is ON)\r\n");
    // Turn off the load relay and switch on the start caps
    digitalWrite(LOAD_RELAY, LOW);
    digitalWrite(IDLER_RELAY, HIGH);
    delay(10);

/*
 * Step 3: If voltage is detected on the idler motor, then
 * turn off the start capcacitors and turn on the load
 * relay to run off the load power source
 */
    D("Step 2: Waiting for Idler Voltage to Rise by taking averages of the start capacitor output Signal.\r\n");
    float idlerState = 0.0;
    do{
      idlerState = readAverage(IDLER_STATE, 1000);
    }while(idlerState < SETPOINT);
    D("Step 3: Ider Voltage has gone up. Turning off Start Caps & switching back to load.\r\n");
    D("\t Idler State: ");
    D(idlerState);
    D("(1 is ON. Less than 1 is OFF)\r\n");

    D("Step 4: Waiting for start capcitors to discharge before running the load.\r\n");
    idlerState = 1.0;
    do{
      idlerState = readAverage(IDLER_STATE, 1000);
    }while(idlerState == SETPOINT);
    D("\t Idler State: ");
    D(idlerState);
    D("(1 is ON. Less than 1 is OFF)\r\n");
    delay(500); // Half-second delay
    digitalWrite(LOAD_RELAY, HIGH);
    digitalWrite(IDLER_RELAY, HIGH);

/*
 * Step 4: If the Raspberry Pi detects low current due to the 
 * user shutting off the equipment, the load circuit
 * will be disconnected and the idler motor will continue to run.
 */
  D("Step 5: Unit will run until load current drops when user powers off their equipment\r\n");
#ifdef PZEM_SENSOR
    do{
      loadSensor = (unsigned int)pzem.current(ip);
      delay(10);
    }while(i > 1.0);
#elif ANALOG_DETECT
    while(analogRead(A0) != 1.0);
#elif ONOFF_DETECT
  // Wait for the Load Sensor Output signal to drop
  float loadState = 0.0f;
  do{
    loadState = readAverage(A0, 1000);
  }while(loadState < 1.0);
#endif

/*
 * Step 5: Shut down sequence
 */
    D("Step 6: Shut Down Sequence.\r\n");
    D("\t Load State: ");
    D(loadState);
    D(" (1 is OFF. Less than 1 is ON)\r\n");
    digitalWrite(LOAD_RELAY, HIGH);
    digitalWrite(IDLER_RELAY, LOW);
    delay(100);
    digitalWrite(LED, LOW);

    D("- Back to Start. Waiting for Load Current -\r\n\r\n");
  }
}

/*
 * Take averages of the readings
 */
float readAverage(int pin, int samples){
  float average = 0;
  for(int i = 0; i<samples; i++){
    #ifdef ANALOG_DETECT
    average += (float)analogRead(pin);
    #elif ONOFF_DETECT
    average += digitalRead(pin);
    #endif
  }
  return average = average/(float)samples;
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
        Serial.print("Relay D6: ");
        Serial.println(relay1);
        break;
      case '2':
        relay2 = !relay2;
        digitalWrite(5, relay2);
        Serial.print("Relay D5: ");
        Serial.println(relay2);
        break;
      case '3':
        relay3 = !relay3;
        digitalWrite(4, relay4);
        Serial.print("Relay D4: ");
        Serial.println(relay3);
        break;
      case '4':
        relay4 = !relay4;
        digitalWrite(3, relay3);
        Serial.print("Relay D3: ");
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
