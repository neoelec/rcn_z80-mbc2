// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"

#define PIN_BANK1         11    // PD3 pin 17   RAM Memory bank address (High)
#define PIN_BANK0         12    // PD4 pin 18   RAM Memory bank address (Low)

static void dev_bank_write(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct dev_bank *bank = container_of(io_dev, struct dev_bank, io_dev);
  struct ios *ios = bank->ios;

  if (ios->io_command != E_IOS_WR_SETBANK)
    return;

  if (bank->num == ios->io_data)
    return;

  bank->num = ios->io_data;

  switch (bank->num) {
    case 0:
      digitalWrite(PIN_BANK0, HIGH);
      digitalWrite(PIN_BANK1, LOW);
      break;
    case 1:
      digitalWrite(PIN_BANK0, HIGH);
      digitalWrite(PIN_BANK1, HIGH);
      break;
    case 2:
      digitalWrite(PIN_BANK0, LOW);
      digitalWrite(PIN_BANK1, HIGH);
      break;
  }

  ios->io_command = E_IOS_NO_OPERATION;
  ios->io_handled = true;
}

static struct dev_bank *__bank_get_instance(void) {
  static struct dev_bank __bank;
  struct dev_bank *bank = &__bank;
  struct e8bit_io_dev *io_dev = &bank->io_dev;

  pinMode(PIN_BANK0, OUTPUT);
  digitalWrite(PIN_BANK0, HIGH);
  pinMode(PIN_BANK1, OUTPUT);
  digitalWrite(PIN_BANK1, LOW);

  bank->num = 0;

  io_dev->write = dev_bank_write;

  return bank;
}

struct dev_bank *dev_bank_get_instance(void) {
  static struct dev_bank *bank;

  if (bank)
    return bank;

  bank = __bank_get_instance();

  return bank;
}

void dev_bank_install(struct ios *ios) {
  struct dev_bank *user = dev_bank_get_instance();

  user->ios = ios;

  e8bit_io_attach_dev(&ios->io, &user->io_dev);
}
