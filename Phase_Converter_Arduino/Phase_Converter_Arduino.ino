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
 * SETTINGS
 */
#define DEBUG //Defines whether debug output will be used. Comment out if not using 
//#define PZEM_SENSOR 1
//#define ANALOG_DETECT 1
#define ONOFF_DETECT  1
 
#ifdef DEBUG 
  #define D(x) Serial.print(x)
#else
  #define D(x)
#endif

// Include Libraries
#ifdef PZEM_SENSOR
#include <PZEM004T.h>
#include <SoftwareSerial.h>
PZEM004T pzem(12,11);  // (RX,TX) connect to TX,RX of PZEM
IPAddress ip(192,168,1,1);
#endif

// Arduino Pin Definitions
const byte IDLER_VTG = A0;          //Analog Input To Pin A0
const byte RPIGPIO_20_START = A1;   //Input <- From Rapsberry PI GPIO20
const byte RPIGPIO_21_FINISH = A2;  //Input <- From Rapsberry PI GPIO21
const byte IDLER_STATE = 10;        //Input <- To Pin D10
const byte UNUSED_RELAY = 6;        //Output -> To Pin D6 
const byte LOAD_RELAY = 3;          //Output -> To Pin D3
const byte IDLER_RELAY = 4;         //Output -> To Pin D5
const byte STARTCAP_RELAY = 5;      //Output -> To Pin D4
const byte LED = 13;                //Output -> To Pin D13 Arduino Onboard LED

// Set Constants used for triggering values
const int IDLER_ON_LIMIT = 0; //VAC
const int IDLER_OFF_LIMIT = 0; //VAC
const float SETPOINT = 1.0;

// Global Variables
unsigned int loadSensor = 0; // Value of the Load Current used to trigger relay sequence

// Set Up Routine.Runs first in the Program
void setup() {
  //Testing Mode. Comment out when not in use
  //testmode(); 

  // Start Arduino to PC Serial Comm
  Serial.begin(9600);
  
  // Set Arduino GPIO pins to Output
  pinMode(RPIGPIO_20_START, INPUT);
  pinMode(RPIGPIO_21_FINISH, INPUT);
  pinMode(LOAD_RELAY, OUTPUT);
  pinMode(STARTCAP_RELAY, OUTPUT);
  pinMode(IDLER_RELAY, OUTPUT);
  pinMode(UNUSED_RELAY, OUTPUT);
  pinMode(IDLER_STATE, INPUT);
  pinMode(LED, OUTPUT);
  
  // Set Default States for the Relays
  digitalWrite(LOAD_RELAY, HIGH);   // EXTERNAL EQUIPMENT. NORMALLY CLOSED
  digitalWrite(STARTCAP_RELAY, LOW);  // START CAPACITORS
  digitalWrite(IDLER_RELAY, LOW);  // IDLER MOTOR
  digitalWrite(UNUSED_RELAY, LOW);  // Unused Relay
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
  loadSensor = digitalRead(A0);
#endif
  D(loadSensor + "\r\n");

/*
 * Step 2: Check if Load amperage is detected
 */
  if(loadSensor >= SETPOINT || loadSensor == true){
    digitalWrite(LED, HIGH);
    D("Step 1: Shut down load. Start Capaciter & Idler\r\n");
    // Turn off the load relay and switch on the start caps
    digitalWrite(LOAD_RELAY, LOW);
    digitalWrite(IDLER_RELAY, HIGH);
    delay(10);

/*
 * Step 3: If voltage is detected on the idler motor, then
 * turn off the start capcacitors and turn on the load
 * relay to run off the load power source
 */
    D("Step 2: Waiting for Idler Voltage to Rise\r\n");
    while(digitalRead(IDLER_STATE) != true)
      delay(10);
    delay(500);
    D("Step 4: Ider Voltage has gone up. Turning off Start Caps & switching back to load\r\n");
    digitalWrite(LOAD_RELAY, HIGH);
    
/*
 * Step 4: If the Raspberry Pi detects low current due to the 
 * user shutting off the equipment, the load circuit
 * will be disconnected and the idler motor will continue to run.
 */
#ifdef PZEM_SENSOR
    do{
      loadSensor = (unsigned int)pzem.current(ip);
      D(loadSensor + "\r\n");
      delay(10);
    }while(i > 1.0);
#elif ANALOG_DETECT
    while(analogRead(A0) <= SETPOINT);
#elif ONOFF_DETECT
    while(digitalRead(A0) != false);
#endif

/*
 * Step 5: Shut down sequence
 */
    D("Step 5: Shut Down Sequence.\r\n");
    digitalWrite(LOAD_RELAY, HIGH);
    digitalWrite(IDLER_RELAY, LOW);
    delay(100);
    digitalWrite(LED, LOW);
  }
}

/*
 * Take analog readings and average them
 */
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
