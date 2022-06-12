// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"
#include "inc/ios.h"
#include "inc/z80mbc.h"

static void z80mbc_run(struct ios *ios);

void setup() {
  struct ios *ios;
  struct dev_tty *tty;
  struct dev_user *user;

  tty = dev_tty_get_instance(); // init tty 1st to read logs
  user = dev_user_get_instance();

  z80mbc_util_setup();
  ios = ios_get_instance();
  dev_install(ios);

  if (dev_user_get_boot_selection(user))
    z80mbc_state_set(z80mbc_state_menu_load);
  else
    z80mbc_state_set(z80mbc_bl_load);
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

static void z80mbc_run(struct ios *ios) {
  ios_loop(ios);
}
