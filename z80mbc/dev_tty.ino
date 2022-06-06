// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"

#define PIN_NMCU_nRTS     23    // PC7 pin 29   * RESERVED - NOT USED *
#define PIN_NMCU_CTS      10    // PD2 pin 16   * RESERVED - NOT USED *

static void dev_tty_write(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct dev_tty *tty = container_of(io_dev, struct dev_tty, io_dev);
  struct ios *ios = tty->ios;

  if (ios->io_command != E_IOS_WR_SERIAL_TX)
    return;

  Serial.write(ios->io_data);

  ios->io_command = E_IOS_NO_OPERATION;
  ios->io_handled = true;
}

static inline void __tty_reset_uterm(void) {
  // Initialize MCU_RTS and MCU_CTS and reset uTerm (A071218-R250119) if present
  pinMode(PIN_NMCU_CTS, INPUT_PULLUP);  // Parked (not used)
  pinMode(PIN_NMCU_nRTS, OUTPUT);
  digitalWrite(PIN_NMCU_nRTS, LOW); // Reset uTerm (A071218-R250119)
  delay(100);
  digitalWrite(PIN_NMCU_nRTS, HIGH);
  delay(500);
}

static struct dev_tty *__tty_get_instance(void) {
  static struct dev_tty __tty;
  struct dev_tty *tty = &__tty;
  struct e8bit_io_dev *io_dev = &tty->io_dev;

  Serial.begin(115200);
  __tty_reset_uterm();
  Serial.println();

  io_dev->write = dev_tty_write;

  return tty;
}

static void __tty_print_banner(void) {
  Serial.println();
  Serial.println(F("Z80-MBC2 - A040618 "));
  Serial.println(F("IOS - I/O Subsystem - S220718-R240620"));
  Serial.println(F("    - Raccoon's MOD"));
  Serial.println();
}

struct dev_tty *dev_tty_get_instance(void) {
  static struct dev_tty *tty;

  if (tty)
    return tty;

  tty = __tty_get_instance();

  return tty;
}

void dev_tty_install(struct ios *ios) {
  struct dev_tty *user = dev_tty_get_instance();

  user->ios = ios;

  e8bit_io_attach_dev(&ios->io, &user->io_dev);

  __tty_print_banner();
}
