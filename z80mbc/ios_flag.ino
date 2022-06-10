// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/ios.h"

void ios_flag_rtc_en_set(struct ios_flag *flag, uint8_t rtc_en) {
  flag->rtc_en = rtc_en;
}

void ios_flag_tty_rx_last_empty_set(struct ios_flag *flag, uint8_t rx_last_empty) {
  flag->rx_last_empty = rx_last_empty;
}

static void ios_flag_read(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct ios_flag *flag = container_of(io_dev, struct ios_flag, io_dev);
  struct ios *ios = container_of(flag, struct ios, flag);

  if (ios->io_command != E_IOS_RD_SYSFLAGS)
    return;

  flag->rx_available = !!Serial.available();

  ios->io_data = flag->io_data;
  ios->io_command = E_IOS_NO_OPERATION;
}

static void __flag_autoexec_en_setup(struct ios_flag *flag) {
  struct ios *ios = container_of(flag, struct ios, flag);

  flag->autoexec_en = ios_cfg_get_autoexec_en(&ios->cfg);
}

void ios_flag_setup(struct ios_flag *flag) {
  struct ios *ios = container_of(flag, struct ios, flag);
  struct e8bit_io_dev *io_dev = &flag->io_dev;

  __flag_autoexec_en_setup(flag);

  io_dev->read = ios_flag_read;

  e8bit_io_attach_dev(&ios->io, &flag->io_dev);
}
