// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/ios.h"
#include "inc/z80mbc.h"

#define PIN_CLK           15    // PD7 pin 21   Z80 CLK

void ios_clk_set_prescale(uint16_t scale) {
  OCR2 = static_cast<uint8_t>(scale & 0xFF);
}

void ios_clk_enable(void) {
  TCCR2 |= _BV(COM20);
  TCCR2 &= ~_BV(COM21);
}

void ios_clk_disable(void) {
  TCCR2 &= ~_BV(COM20);
  TCCR2 &= ~_BV(COM21);
}

void ios_clk_pulse(size_t n) {
  size_t i;

  for (i = 0; i < n; i++) {
    digitalWrite(PIN_CLK, HIGH);
    asm volatile ("nop");
    digitalWrite(PIN_CLK, LOW);
    asm volatile ("nop");
  }
}

void ios_clk_setup(void) {
  ASSR &= ~_BV(AS2);
  TCCR2 |= _BV(CS20);
  TCCR2 &= ~(_BV(CS21) | _BV(CS22));
  TCCR2 |= _BV(WGM21);
  TCCR2 &= ~_BV(WGM20);

  ios_clk_disable();

  pinMode(PIN_CLK, OUTPUT);
  digitalWrite(PIN_CLK, HIGH);
}
