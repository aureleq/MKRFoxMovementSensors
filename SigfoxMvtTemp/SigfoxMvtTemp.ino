/*
  Sigfox movement detector and temperature/humidity reporting
  Based on MKRFox Movement trigger project: https://create.arduino.cc/projecthub/45374/mkr-fox-1200-movement-trigger-dacbe0

  Temperature and Humidity are reported periodically (SLEEPTIME value) or when a movement is detected
  
  Components required
    - MKRFox 1200
    - MMA8451
    - DHT11
    - 10k resistor
  
  Refer to MKR_DHT11_Accelero_Schematics_bb.png for connections 
*/

#include <SigFox.h>
#include <ArduinoLowPower.h>
#include <DHT.h>
#include "MMA8451_IRQ.h"

#define DHTPIN        1                // What digital pin we're connected to
#define DHTTYPE       DHT11
//#define DEBUG                      // Uncomment to enable serial prints during setup
#define SLEEPTIME     10 * 60 * 1000   // Set the delay to 10 minutes (10 min x 60 seconds x 1000 milliseconds)

#define UINT16_t_MAX  65536
#define INT16_t_MAX   UINT16_t_MAX/2


// Structure to pack sigfox message
typedef struct __attribute__ ((packed)) sigfox_message {
        uint8_t movement;
        int8_t dhtTemperature;
        uint8_t dhtHumidity;
} SigfoxMessage;

// Stub for message which will be sent
SigfoxMessage msg;

// Declare sensors
DHT dht(DHTPIN, DHTTYPE);
MMA8451_IRQ mma = MMA8451_IRQ();

volatile bool alarmOccurred = false;

void setup() {

  #ifdef DEBUG
  Serial.begin(9600);
  while (!Serial) {};
  #endif DEBUG
  
  if (!SigFox.begin()) {
    //something is really wrong, try rebooting
    #ifdef DEBUG
    Serial.println("Cannot Start SX module!");
    #endif
    reboot();
  }  
  //Enable DEBUG prints and LED indication  
  SigFox.debug();

  //Send module to standby until we need to send a message
  SigFox.end();

  if (! mma.begin()) {
    #ifdef DEBUG
    Serial.println("Cannot Start MMA Accelero!");
    #endif
    reboot();
  }

  // init DHT sensor
  dht.begin();
  // Setup the I2C accelerometer
  mma.enableInterrupt();

  // attach pin 0 to accelerometer INT1 and enable the interrupt on voltage falling event
  pinMode(0, INPUT_PULLUP);
  LowPower.attachInterruptWakeup(0, alarmEvent, FALLING);

  // attach RTC interrupt to wakeup periodially
  LowPower.attachInterruptWakeup(RTC_ALARM_WAKEUP, dummyEvent, CHANGE);
}


void loop()
{
  // Sleep until delay exceeded or movement detected
  LowPower.sleep(SLEEPTIME);

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    return;
  }

  msg.dhtTemperature = (int8_t) t;
  msg.dhtHumidity = (uint8_t) h;


  SigFox.begin();
  // Wait at least 30ms after first configuration (100ms before)
  delay(100);

  SigFox.beginPacket();
  SigFox.write((uint8_t*)&msg, 3);
  int ret = SigFox.endPacket();

  delay(100);
  mma.clearInterrupt();
  delay(100);

  // shut down module, back to standby
  SigFox.end();

  alarmOccurred = false;
  msg.movement = 0;
}

// function called when interrupt on pin 0 occurs
void alarmEvent() {
  alarmOccurred = true;
  msg.movement = 1;
}

// dummy fonction to capture RTC interrupt
void dummyEvent() {
}

void reboot() {
  NVIC_SystemReset();
  while (1);
}

