// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/ios.h"

#define PIN_nMREQ         21    // PC5 pin 27   Z80 MREQ
#define PIN_nRD           20    // PC4 pin 26   Z80 RD
#define PIN_nWR           19    // PC3 pin 25   Z80 WR

uint8_t ios_sys_get_nMREQ(void) {
  return digitalRead(PIN_nMREQ);
}

uint8_t ios_sys_get_nRD(void) {
  return digitalRead(PIN_nRD);
}

uint8_t ios_sys_get_nWR(void) {
  return digitalRead(PIN_nWR);
}

void ios_sys_setup(void) {
  pinMode(PIN_nMREQ, INPUT_PULLUP);
  pinMode(PIN_nRD, INPUT_PULLUP);
  pinMode(PIN_nWR, INPUT_PULLUP);
}
