// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"
#include "inc/ios.h"
#include "inc/z80mbc.h"

static void __bl_inject_to_ram(uint8_t data) {
  const uint8_t op_LD_HL_n = 0x36;
  const uint8_t op_INC_HL = 0x23;

  ios_clk_pulse(1);
  ios_bus_set_RAM_CE2_LOW();
  ios_bus_write(op_LD_HL_n);
  ios_clk_pulse(2);
  ios_bus_release();
  ios_clk_pulse(2);

  ios_bus_write(data);
  ios_clk_pulse(2);
  ios_bus_release();
  ios_bus_set_RAM_CE2_HIGH();
  ios_clk_pulse(3);

  ios_clk_pulse(1);
  ios_bus_set_RAM_CE2_LOW();
  ios_bus_write(op_INC_HL);
  ios_clk_pulse(2);
  ios_bus_release();
  ios_bus_set_RAM_CE2_HIGH();
  ios_clk_pulse(3);
}

static void __bl_inject_instr_LD_HL_nn(uint16_t nn) {
  const uint8_t op_LD_HL_nn = 0x21;

  ios_clk_pulse(1);
  ios_bus_set_RAM_CE2_LOW();
  ios_bus_write(op_LD_HL_nn);
  ios_clk_pulse(2);
  ios_bus_release();
  ios_clk_pulse(2);

  ios_bus_write(lowByte(nn));
  ios_clk_pulse(3);

  ios_bus_write(highByte(nn));
  ios_clk_pulse(2);
  ios_bus_release();
  ios_bus_set_RAM_CE2_HIGH();
}

static void __bl_hard_reset(void) {
  ios_cpu_set_nRESET_LOW();
  ios_clk_pulse(6);
  ios_cpu_set_nRESET_HIGH();
  ios_clk_pulse(2);
}

static void __bl_set_mem_addr(uint16_t mem_addr) {
  const uint8_t op_JP_nn = 0xC3;

  __bl_inject_instr_LD_HL_nn(0x0000);

  if (mem_addr) {
    __bl_inject_to_ram(op_JP_nn);
    __bl_inject_to_ram(lowByte(mem_addr));
    __bl_inject_to_ram(highByte(mem_addr));

    __bl_inject_instr_LD_HL_nn(mem_addr);
  }
}

static void __bl_load_file_image(struct ios_cfg *cfg) {
  struct dev_sd *sd;
  ssize_t err;
  ssize_t sz_read;
  ssize_t i;

  sd = dev_sd_get_instance();
  err = dev_sd_open_raw(sd, cfg->boot_file);
  if (err < 0)
    dev_sd_hang_on_err(err, E_DEV_SD_OPEN, cfg->boot_file);

  __bl_set_mem_addr(cfg->base_addr);

  do {
    sz_read = dev_sd_read_raw(sd, sd->buf, DEV_SD_BUF_SZ);
    if (sz_read < 0)
      dev_sd_hang_on_err(err, E_DEV_SD_READ, cfg->boot_file);

    for (i = 0; i < sz_read; i++)
      __bl_inject_to_ram(sd->buf[i]);
  } while (sz_read == DEV_SD_BUF_SZ);

  __bl_set_mem_addr(cfg->boot_addr);
}

static void __bl_flush_rx(void) {
  while (Serial.available())
    uint8_t dummy = Serial.read();
}

void z80mbc_state_bl_load(struct ios *ios) {
  ios_cfg_print_cfg(&ios->cfg);
  Serial.println();

  z80mbc_state_set(z80mbc_state_bl_run);
}

void z80mbc_state_bl_run(struct ios *ios) {
  struct ios_cfg *cfg = &ios->cfg;
  static const char load_fmt[] PROGMEM = "MBC: Loading %s@0x%04X ...\n";

  __bl_hard_reset();
  ios_cpu_set_nWAIT_HIGH();

  z80mbc_printf_P(load_fmt, cfg->boot_file, cfg->base_addr);
  __bl_load_file_image(cfg);
  Serial.println(F("     ... Done."));

  z80mbc_state_set(z80mbc_state_bl_epilog);
}

void z80mbc_state_bl_epilog(struct ios *ios) {
  __bl_hard_reset();
  ios_cpu_set_nWAIT_HIGH();
  ios_cpu_set_nRESET_LOW();
  ios_clk_enable();
  delay(1);                     // Just to be sure...
  __bl_flush_rx();
  ios_cpu_set_nRESET_HIGH();

  z80mbc_state_set(z80mbc_state_ios_run);
}
