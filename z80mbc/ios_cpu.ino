// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/ios.h"

#define PIN_nWAIT         3     // PB3 pin 4    Z80 WAIT
#define PIN_nWAIT_RES     0     // PB0 pin 1    Reset the Wait FF
#define PIN_nINT          1     // PB1 pin 2    Z80 control bus
#define PIN_nRESET        22    // PC6 pin 28   Z80 RESET

uint8_t ios_cpu_get_nWAIT(void) {
  return digitalRead(PIN_nWAIT);
}

void ios_cpu_set_nWAIT_HIGH(void) {
  digitalWrite(PIN_nWAIT_RES, HIGH);
}

void ios_cpu_set_nWAIT_LOW(void) {
  digitalWrite(PIN_nWAIT_RES, LOW);
}

void ios_cpu_set_nINT_HIGH(void) {
  digitalWrite(PIN_nINT, HIGH);
}

void ios_cpu_set_nINT_LOW(void) {
  digitalWrite(PIN_nINT, LOW);
}

void ios_cpu_set_nRESET_HIGH(void) {
  digitalWrite(PIN_nRESET, HIGH);
}

void ios_cpu_set_nRESET_LOW(void) {
  digitalWrite(PIN_nRESET, LOW);
}

void ios_cpu_setup(void) {
  pinMode(PIN_nWAIT, INPUT);
  pinMode(PIN_nWAIT_RES, OUTPUT);
  digitalWrite(PIN_nWAIT_RES, LOW);

  pinMode(PIN_nINT, OUTPUT);
  digitalWrite(PIN_nINT, HIGH);

  pinMode(PIN_nRESET, OUTPUT);
  digitalWrite(PIN_nRESET, LOW);
}
