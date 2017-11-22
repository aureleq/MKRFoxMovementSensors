#include "MMA8451_IRQ.h"

void MMA8451_IRQ::enableInterrupt() {

  // Clear any stray interrupt
  read();
  Serial.println(readRegister8(MMA8451_REG_CTRL_REG4));
  Serial.println(readRegister8(MMA8451_REG_CTRL_REG5));
  writeRegister8(MMA8451_REG_CTRL_REG2, 0x40); // reset
  while (readRegister8(MMA8451_REG_CTRL_REG2) & 0x40);

  // Put the device into Standby Mode
  writeRegister8(0x2A, 0x18);
  // Set Configuration Register for Motion Detection
  writeRegister8(0x15, 0xDB);
  // Threshold Setting Value for the Motion detection of > 2G
  writeRegister8(0x17, 0x24);
  // Set the debounce counter to eliminate false readings for 100 Hz sample rate with a requirement of 100 ms timer
  writeRegister8(0x18, 0x0A);
  // Enable Motion/Freefall Interrupt Function in the System 
  writeRegister8(0x2D, 0x04);
  // Route the Motion/Freefall Interrupt Function to INT1 hardware pin
  writeRegister8(0x2E, 0x04);
  // Put the device in Active Mode
  writeRegister8(MMA8451_REG_CTRL_REG1, 0x01 | 0x04);
}

byte MMA8451_IRQ::clearInterrupt() {
  // Determine source of interrupt by reading the system interrupt
  uint8_t reg = readRegister8(0xC);
  if ((reg & 0x04) == 0x04) {
    // Read the Motion/Freefall Function to clear the interrupt
    readRegister8(0x16);
  }
  return reg;
}

