/*
  SigFox mobility alarm system

  This sketch demonstrates the usage of a MKRFox1200
  to build a battery-powered alarm sensor with email notifications

  An I2C accelerometer will trigger an interrupt when enough motion is detected.
  This interrupt will wake up the chip and and a message to the owner

  This example uses http://librarymanager/all#Adafruit&MMA845 library

  Depends on https://github.com/adafruit/Adafruit_MMA8451_Library/pull/13

  This example code is in the public domain.
*/

#include <SigFox.h>
#include <ArduinoLowPower.h>
#include "MMA8451_IRQ.h"

MMA8451_IRQ mma = MMA8451_IRQ();

volatile bool alarmOccurred = false;

void setup() {

  if (!SigFox.begin()) {
    //something is really wrong, try rebooting
    reboot();
  }

  SigFox.debug();

  //Send module to standby until we need to send a message
  SigFox.end();

  if (! mma.begin()) {
    reboot();
  }

  // Setup the I2C accelerometer
  mma.enableInterrupt();

  // attach pin 0 to accelerometer INT1 and enable the interrupt on voltage falling event
  pinMode(0, INPUT_PULLUP);
  LowPower.attachInterruptWakeup(0, alarmEvent, FALLING);
}

void loop()
{
  // Sleep until an event is recognized
  LowPower.sleep();

  // if we get here it means that an event was received

  if (alarmOccurred == false) {
    return;
  }

  SigFox.begin();

  delay(100);

  String to_be_sent = "MOVING!";

  SigFox.beginPacket();
  SigFox.print(to_be_sent);
  int ret = SigFox.endPacket();

  delay(100);

  mma.clearInterrupt();

  delay(100);

  // shut down module, back to standby
  SigFox.end();

  alarmOccurred = false;
}

void alarmEvent() {
  alarmOccurred = true;
}

void reboot() {
  NVIC_SystemReset();
  while (1);
}

