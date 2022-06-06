// SPDX-License-Identifier: GPL-3.0-or-later
#include <stdarg.h>

#include "inc/z80mbc.h"

#define PIN_LED_IOS       0     // PB0 pin 1    Led PIN_LED_IOS is ON if HIGH

void z80mbc_putstr_P(const char *str) {
  size_t i;

  for (i = 0; i < strlen_P(str); i++) {
    char c = pgm_read_byte_near(str + i);

    Serial.print(c);
  }
}

void z80mbc_putstrln_P(const char *str) {
  z80mbc_putstr_P(str);

  Serial.println();
}

#ifndef PRINTF_BUF_SZ
#define PRINTF_BUF_SZ   128
#endif

void z80mbc_printf_P(const char *fmt, ...) {
  char buf[PRINTF_BUF_SZ];      // resulting string limited to 128 chars
  char *__buf = buf;
  va_list args;

  va_start(args, fmt);
#ifdef __AVR__
  vsnprintf_P(buf, sizeof(buf), (const char *)fmt, args); // progmem for AVR
#else
  vsnprintf(buf, sizeof(buf), (const char *)fmt, args); // for the rest of the world
#endif
  va_end(args);

  while (*__buf) {
    Serial.write(*__buf);
    if (*__buf == '\n')
      Serial.write('\r');

    __buf++;
  }
}

void z80mbc_blink_ios_led(void) {
  static unsigned long last_called;
  unsigned long current_called = millis();

  if ((current_called - last_called) > 200) {
    digitalWrite(PIN_LED_IOS, !digitalRead(PIN_LED_IOS));
    last_called = current_called;
  }
}

void z80mbc_util_setup(void) {
  pinMode(PIN_LED_IOS, OUTPUT);
}
