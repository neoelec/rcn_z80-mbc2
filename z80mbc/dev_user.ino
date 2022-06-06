// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"

#define PIN_USER          13    // PD5 pin 19   Led USER and key (led USER is ON if LOW)

static uint8_t __user_read_key(struct dev_user *user) {
  uint8_t key_status;

  user->led_status = digitalRead(PIN_USER);

  pinMode(PIN_USER, INPUT_PULLUP);
  key_status = !digitalRead(PIN_USER);

  pinMode(PIN_USER, OUTPUT);
  digitalWrite(PIN_USER, user->led_status);

  return key_status;
}

static void __user_led_on(struct dev_user *user) {
  pinMode(PIN_USER, OUTPUT);
  digitalWrite(PIN_USER, LOW);
  user->led_status = LOW;
}

static void __user_led_off(struct dev_user *user) {
  pinMode(PIN_USER, OUTPUT);
  digitalWrite(PIN_USER, HIGH);
  user->led_status = HIGH;
}

uint8_t dev_user_get_boot_selection(struct dev_user *user) {
  return __user_read_key(user);
}

static void dev_user_read(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct dev_user *user = container_of(io_dev, struct dev_user, io_dev);
  struct ios *ios = user->ios;

  if (ios->io_command != E_IOS_RD_USER_KEY)
    return;

  ios->io_data = __user_read_key(user);
  ios->io_command = E_IOS_NO_OPERATION;
  ios->io_handled = true;
}

static void dev_user_write(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct dev_user *user = container_of(io_dev, struct dev_user, io_dev);
  struct ios *ios = user->ios;

  if (ios->io_command != E_IOS_WR_USER_LED)
    return;

  if (ios->io_data & 0x01)
    __user_led_on(user);
  else
    __user_led_off(user);

  ios->io_command = E_IOS_NO_OPERATION;
  ios->io_handled = true;
}

static struct dev_user *__user_get_instance(void) {
  static struct dev_user __user;
  struct dev_user *user = &__user;
  struct e8bit_io_dev *io_dev = &user->io_dev;

  __user_led_off(user);

  io_dev->read = dev_user_read;
  io_dev->write = dev_user_write;

  return user;
}

struct dev_user *dev_user_get_instance(void) {
  static struct dev_user *user;

  if (user)
    return user;

  user = __user_get_instance();

  return user;
}

void dev_user_install(struct ios *ios) {
  struct dev_user *user = dev_user_get_instance();

  user->ios = ios;

  e8bit_io_attach_dev(&ios->io, &user->io_dev);
}
