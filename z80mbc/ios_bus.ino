// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/ios.h"

#define PIN_nBUSREQ       14    // PD6 pin 20   Z80 BUSRQ
#define PIN_A0            18    // PC2 pin 24   Z80 A0
#define PIN_RAM_CE2       2     // PB2 pin 3    RAM Chip Enable (CE2). Active HIGH. Used only during boot

void ios_bus_set_nBUSREQ_HIGH(void) {
  digitalWrite(PIN_nBUSREQ, HIGH);
}

void ios_bus_set_nBUSREQ_LOW(void) {
  digitalWrite(PIN_nBUSREQ, LOW);
}

uint8_t ios_bus_get_A0(void) {
  return digitalRead(PIN_A0);
}

void ios_bus_set_RAM_CE2_HIGH(void) {
  digitalWrite(PIN_RAM_CE2, HIGH);
}

void ios_bus_set_RAM_CE2_LOW(void) {
  digitalWrite(PIN_RAM_CE2, LOW);
}

uint8_t ios_bus_read(void) {
  return static_cast<uint8_t>(PINA & 0xFF);
}

void ios_bus_write(uint8_t data) {
  DDRA = 0xFF;
  PORTA = data;
}

void ios_bus_release(void) {
  DDRA = 0x00;
  PORTA = 0xFF;
}

void ios_bus_setup(void) {
  pinMode(PIN_nBUSREQ, OUTPUT);
  digitalWrite(PIN_nBUSREQ, HIGH);

  pinMode(PIN_A0, INPUT_PULLUP);
  pinMode(PIN_RAM_CE2, OUTPUT);
  digitalWrite(PIN_RAM_CE2, HIGH);
  ios_bus_release();
}
