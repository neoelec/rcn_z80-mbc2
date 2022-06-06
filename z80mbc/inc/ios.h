#ifndef __IOS_H__
#define __IOS_H__

#include <stdint.h>

#include <emul_8bit.h>

#include "ios_bus.h"
#include "ios_cfg.h"
#include "ios_clk.h"
#include "ios_cpu.h"
#include "ios_flag.h"
#include "ios_sys.h"

enum {
  E_IOS_WR_USER_LED,
  E_IOS_WR_SERIAL_TX,
  E_IOS_WR_GPIOA = 0x03,
  E_IOS_WR_GPIOB,
  E_IOS_WR_IODIRA,
  E_IOS_WR_IODIRB,
  E_IOS_WR_GPPUA,
  E_IOS_WR_GPPUB,
  E_IOS_WR_SELDISK,
  E_IOS_WR_SELTRACK,
  E_IOS_WR_SELSECT,
  E_IOS_WR_WRITESECT,
  E_IOS_WR_SETBANK,
  E_IOS_RD_USER_KEY = 0x80,
  E_IOS_RD_GPIOA,
  E_IOS_RD_GPIOB,
  E_IOS_RD_SYSFLAGS,
  E_IOS_RD_DATETIME,
  E_IOS_RD_ERRDISK,
  E_IOS_RD_READSECT,
  E_IOS_RD_SDMOUNT,
  E_IOS_NO_OPERATION = 0xFF,
};

struct ios {
  struct e8bit_io io;
  struct e8bit_bus bus;
  uint8_t io_data;
  uint8_t io_command;
  bool io_handled;
  size_t io_count;
  uint8_t io_area[1];
  struct ios_cfg cfg;
  struct ios_flag flag;
};

extern struct ios *ios_get_instance(void);
extern void ios_loop(struct ios *ios);

#endif /* __IOS_H__ */
