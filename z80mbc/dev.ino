// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"

void dev_install(struct ios *ios) {
  dev_tty_install(ios);
  dev_sd_install(ios);
  dev_bank_install(ios);
  dev_gpio_install(ios);
  dev_rtc_install(ios);
  dev_user_install(ios);
}
