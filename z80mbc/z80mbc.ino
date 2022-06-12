// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"
#include "inc/ios.h"
#include "inc/z80mbc.h"

static inline void __tty_eraly_console(void) {
  struct dev_tty *tty;

  tty = dev_tty_get_instance(); // init tty 1st to read logs
}

static inline void __ios_setup(void) {
  struct ios *ios;

  z80mbc_util_setup();
  ios = ios_get_instance();
  dev_install(ios);
}

static inline void __state_setup(void) {
  struct dev_user *user;

  user = dev_user_get_instance();

  if (dev_user_get_boot_selection(user))
    z80mbc_state_set(z80mbc_state_menu_load);
  else
    z80mbc_state_set(z80mbc_bl_load);
}

void setup() {
  __tty_eraly_console();
  __ios_setup();
  __state_setup();
}

void loop() {
  struct ios *ios = ios_get_instance();

  z80mbc_state_machine(ios);
}

void serialEvent() {
  struct ios *ios = ios_get_instance();

  if (Serial.available() && ios->cfg.use_interrupt)
    ios_cpu_set_nINT_LOW();
}
