// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdarg.h>

#include "inc/z80mbc.h"

#define PIN_LED_IOS       0     // PB0 pin 1    Led PIN_LED_IOS is ON if HIGH

void z80mbc_blink_ios_led(void) {
  static unsigned long last_called;
  unsigned long current_called = millis();

  if ((current_called - last_called) > 200) {
    digitalWrite(PIN_LED_IOS, digitalRead(PIN_LED_IOS) ? LOW : HIGH);
    last_called = current_called;
  }
}

void z80mbc_util_setup(void) {
  pinMode(PIN_LED_IOS, OUTPUT);
}
