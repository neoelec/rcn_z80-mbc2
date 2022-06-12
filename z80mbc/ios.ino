// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/ios.h"

static struct ios *__ios_get_instance(void) {
  static struct ios __ios;
  struct ios *ios = &__ios;
  struct e8bit_bus *bus = &ios->bus;

  e8bit_io_setup(&ios->io, 0, 1, ios->io_area);

  ios->io_data = 0x0;
  ios->io_command = E_IOS_NO_OPERATION;
  ios->io_count = 0;

  e8bit_bus_setup(bus);
  e8bit_bus_attach(bus, &ios->io.dev);

  ios_cfg_setup(&ios->cfg);
  ios_flag_setup(&ios->flag);

  ios_bus_setup();
  ios_clk_setup();
  ios_cpu_setup();
  ios_sys_setup();

  return ios;
}

struct ios *ios_get_instance(void) {
  static struct ios *ios;

  if (ios)
    return ios;

  ios = __ios_get_instance();

  return ios;
}

enum {
  IOS_ON_WRITE = 0,
  IOS_ON_READ,
  IOS_ON_IDLE,
};

static inline uint8_t __get_io_type(void) {
  if (!ios_sys_get_nWR())
    return IOS_ON_WRITE;
  else if (!ios_sys_get_nRD())
    return IOS_ON_READ;

  return IOS_ON_IDLE;
}

static inline void __write_io(struct ios *ios, uint8_t io_type) {
  if (io_type != IOS_ON_WRITE)
    return;

  if (ios_bus_get_A0()) {
    ios->io_command = PINA;
    ios->io_count = 0;
  } else {
    ios->io_data = PINA;
    ios->io_handled = false;
    e8bit_bus_write(&ios->bus, 0, 0);
    if (!ios->io_handled)
      ios->io_command = E_IOS_NO_OPERATION;
  }
}

static inline void __read_io(struct ios *ios, uint8_t io_type) {
  if (io_type != IOS_ON_READ)
    return;

  if (ios_bus_get_A0()) {
    if (Serial.available() > 0) {
      ios->io_data = Serial.read();
      ios_flag_tty_rx_last_empty_set(&ios->flag, 0);
    } else {
      ios->io_data = 0xFF;
      ios_flag_tty_rx_last_empty_set(&ios->flag, 1);
    }

    ios_cpu_set_nINT_HIGH();
  } else {
    ios->io_data = 0;
    ios->io_handled = false;
    e8bit_bus_read(&ios->bus, 0, ios->io_area);
    if (!ios->io_handled)
      ios->io_command = E_IOS_NO_OPERATION;
  }

  ios_bus_write(ios->io_data);
}

static inline void __busy_wati_for_reading(void) {
  asm volatile ("nop");
  asm volatile ("nop");
}

static inline void __post_io(struct ios *ios, uint8_t io_type) {
  if (io_type == IOS_ON_READ) {
    ios_bus_set_nBUSREQ_LOW();
    ios_cpu_set_nWAIT_LOW();
    __busy_wati_for_reading();
    ios_bus_release();
    ios_cpu_set_nWAIT_HIGH();
    ios_bus_set_nBUSREQ_HIGH();
  } else {
    ios_bus_set_nBUSREQ_LOW();
    ios_cpu_set_nWAIT_LOW();
    ios_cpu_set_nWAIT_HIGH();
    ios_bus_set_nBUSREQ_HIGH();
  }
}

static inline void __loop(struct ios *ios) {
  uint8_t io_type;

  io_type = __get_io_type();
  __write_io(ios, io_type);
  __read_io(ios, io_type);
  __post_io(ios, io_type);
}

void z80mbc_ios_run(struct ios *ios) {
  if (ios_cpu_get_nWAIT())
    return;

  __loop(ios);
}
