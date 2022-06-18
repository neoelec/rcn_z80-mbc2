// SPDX-License-Identifier: GPL-3.0-or-later
#include <EEPROM.h>

#include "inc/ios.h"


#define CFG_BOOTMODE_FMT  "BOOTMOxx.CSV"

#define CFG_BOOT_MODE     10
#define CFG_AUTOEXEC_EN   12
#define CFG_CLOCK_MODE    13

static void __cfg_set_boot_mode(struct ios_cfg *cfg, uint8_t boot_mode) {
  cfg->boot_mode = boot_mode;

  EEPROM.update(CFG_BOOT_MODE, boot_mode);
}

static void __cfg_set_autoexec_en(struct ios_cfg *cfg, uint8_t autoexec_en) {
  struct ios *ios = container_of(cfg, struct ios, cfg);

  cfg->autoexec_en = autoexec_en;
  ios->flag.autoexec_en = autoexec_en;

  EEPROM.update(CFG_AUTOEXEC_EN, autoexec_en);
}

static void __cfg_set_clock_mode(struct ios_cfg *cfg, uint8_t clock_mode) {
  cfg->clock_mode = clock_mode;

  EEPROM.update(CFG_CLOCK_MODE, clock_mode);
  ios_clk_set_prescale(clock_mode ? 2 : 1);
}

static void __cfg_read_csv(struct ios_cfg *cfg, char *csv, const char *filename) {
  struct dev_sd *sd;
  ssize_t err;
  ssize_t sz_read;
  size_t pos = 0;

  sd = dev_sd_get_instance();
  err = dev_sd_open_raw(sd, filename);
  if (err < 0)
    dev_sd_hang_on_err(err, E_DEV_SD_OPEN, filename);

  do {
    sz_read = dev_sd_read_raw(sd, sd->buf, sizeof(sd->buf));
    if (sz_read < 0)
      break;

    memcpy(&csv[pos], sd->buf, sz_read);
    pos += sz_read;
  } while (sz_read == sizeof(sd->buf));
}

#define __terminate_cstr(__str) __str[sizeof(__str) - 1] = '\0'

static void ____cfg_parse_csv(struct ios_cfg *cfg, char *token, size_t i) {
  long tmp;

  switch (i) {
    case 0:                    // boot_name
      strncpy(cfg->boot_name, token, sizeof(cfg->boot_name));
      __terminate_cstr(cfg->boot_name);
      break;
    case 1:                    // idx_built_in
      tmp = strtol(token, NULL, 10);
      cfg->idx_built_in = static_cast<int8_t>(tmp & 0xFF);
      break;
    case 2:                    // boot_file
      strncpy(cfg->boot_file, token, sizeof(cfg->boot_file));
      __terminate_cstr(cfg->boot_file);
      break;
    case 3:                    // base_addr
      tmp = strtol(token, NULL, 16);
      cfg->base_addr = static_cast<uint16_t>(tmp & 0xFFFF);
      break;
    case 4:                    // boot_addr
      tmp = strtol(token, NULL, 16);
      cfg->boot_addr = static_cast<uint16_t>(tmp & 0xFFFF);
      break;
    case 5:                    // disk_set
      tmp = strtol(token, NULL, 10);
      cfg->disk_set = static_cast<int8_t>(tmp & 0xFF);
      break;
    case 6:                    // use_irq_tty_rx
      tmp = strtol(token, NULL, 10);
      cfg->use_irq_tty_rx = static_cast<uint8_t>(tmp & 0xFF);
      break;
  }
}

static void __cfg_parse_csv(struct ios_cfg *cfg, char *csv) {
  static const char *delim = ",";
  char *ptr = csv;
  char *token;
  size_t i = 0;

  token = strsep(&ptr, delim);
  while (token) {
    ____cfg_parse_csv(cfg, token, i++);
    token = strsep(&ptr, delim);
  }
}

static void __cfg_print_csv(const struct ios_cfg *cfg) {
  static const char fw_fmt_sd[] PROGMEM = "IOS: FW - %s (%s)";
  static const char fw_fmt_bi[] PROGMEM = "IOS: FW - %s";
  static const char disk_set_fmt[] PROGMEM = " / Disk-Set - %d\n";
  static const char addr_fmt[] PROGMEM = "IOS: BASE@0x%04X / BOOT@%04X\n";

  if (cfg->idx_built_in < 0)
    z80mbc_printf_P(fw_fmt_sd, cfg->boot_name, cfg->boot_file);
  else
    z80mbc_printf_P(fw_fmt_bi, cfg->boot_name);

  if (cfg->disk_set < 0)
    Serial.println();
  else
    z80mbc_printf_P(disk_set_fmt, cfg->disk_set);

  Serial.print(F("IOS: IRQ TTY-RX -"));
  if (cfg->use_irq_tty_rx)
    Serial.println(F(" enabled"));
  else
    Serial.println(F(" disabled"));
}

static void __cfg_print_autoexec_en(const struct ios_cfg *cfg) {
  Serial.print(F("IOS: AUTOEXEC -"));
  if (cfg->autoexec_en)
    Serial.println(F(" enabled"));
  else
    Serial.println(F(" disabled"));
}

static void __cfg_print_clk_mode(const struct ios_cfg *cfg) {
  static const char clk_fmt[] PROGMEM = "IOS: CLK - %lu.%03lu MHz";
  unsigned long khz;

  if (cfg->clock_mode)
    khz = F_CPU / 4 / 1000UL;
  else
    khz = F_CPU / 2 / 1000UL;

  z80mbc_printf_P(clk_fmt, khz / 1000UL, khz % 1000UL);
}

void ios_cfg_print_cfg(const struct ios_cfg *cfg) {
  if (!cfg->nr_boot_mode) {
    Serial.println(F("IOS: Missing BOOTMOxx.CFG files!"));
    while (1) ;
  }

  __cfg_print_csv(cfg);
  __cfg_print_autoexec_en(cfg);
  __cfg_print_clk_mode(cfg);
}

static void __cfg_load_from_csv(struct ios_cfg *cfg) {
  char bootmode_file[] = CFG_BOOTMODE_FMT;
  char csv[DEV_SD_BUF_SZ * 2];

  bootmode_file[6] = cfg->boot_mode / 10 + '0';
  bootmode_file[7] = cfg->boot_mode % 10 + '0';

  __cfg_read_csv(cfg, csv, bootmode_file);
  __cfg_parse_csv(cfg, csv);
}

void ios_cfg_print_boot_mode(struct ios_cfg *cfg) {
  static const char bootmode_fmt[] PROGMEM = "%u - %s\n";
  struct ios_cfg dummy;
  size_t i;

  for (i = 0; i < cfg->nr_boot_mode; i++) {
    dummy.boot_mode = i;
    __cfg_load_from_csv(&dummy);
    z80mbc_printf_P(bootmode_fmt, i, dummy.boot_name);
  }
}

uint8_t ios_cfg_get_nr_boot_mode(struct ios_cfg *cfg) {
  return cfg->nr_boot_mode;
}


uint8_t ios_cfg_get_boot_mode(struct ios_cfg *cfg) {
  return cfg->boot_mode;
}

void ios_cfg_set_boot_mode(struct ios_cfg *cfg, uint8_t boot_mode) {
  if (cfg->boot_mode != boot_mode) {
    __cfg_set_boot_mode(cfg, boot_mode);
    __cfg_load_from_csv(cfg);
  }

  __cfg_print_csv(cfg);
}

uint8_t ios_cfg_get_autoexec_en(struct ios_cfg *cfg) {
  return cfg->autoexec_en;
}

void ios_cfg_set_autoexec_en(struct ios_cfg *cfg, uint8_t autoexec_en) {
  if (cfg->autoexec_en != autoexec_en)
    __cfg_set_autoexec_en(cfg, autoexec_en);

  __cfg_print_autoexec_en(cfg);
}

uint8_t ios_cfg_get_clock_mode(struct ios_cfg *cfg) {
  return cfg->clock_mode;
}

void ios_cfg_set_clock_mode(struct ios_cfg *cfg, uint8_t clock_mode) {
  if (cfg->clock_mode != clock_mode)
    __cfg_set_clock_mode(cfg, clock_mode);

  __cfg_print_clk_mode(cfg);
}

static bool __cfg_boot_mode_file_stat(struct ios_cfg *cfg, uint8_t i) {
  char bootmode_file[] = CFG_BOOTMODE_FMT;
  struct dev_sd *sd;
  ssize_t err;

  bootmode_file[6] = i / 10 + '0';
  bootmode_file[7] = i % 10 + '0';

  sd = dev_sd_get_instance();
  err = dev_sd_open_raw(sd, bootmode_file);
  if (err < 0)
    return false;

  return true;
}

static void __cfg_nr_boot_mode_setup(struct ios_cfg *cfg) {
  uint8_t nr_boot_mode = 0;
  uint8_t i = 0;

  while (__cfg_boot_mode_file_stat(cfg, i++))
    nr_boot_mode++;

  cfg->nr_boot_mode = nr_boot_mode;
}

static void __cfg_boot_mode_setup(struct ios_cfg *cfg) {
  uint8_t boot_mode = EEPROM.read(CFG_BOOT_MODE);

  if (boot_mode >= cfg->nr_boot_mode)
    boot_mode = 0;

  cfg->boot_mode = boot_mode;
}

static void __cfg_autoexec_en_setup(struct ios_cfg *cfg) {
  uint8_t autoexec_en = EEPROM.read(CFG_AUTOEXEC_EN);

  if (autoexec_en > 1) {
    autoexec_en = 0;
    __cfg_set_autoexec_en(cfg, autoexec_en);
  }

  cfg->autoexec_en = autoexec_en;
}

static void __cfg_clock_mode_setup(struct ios_cfg *cfg) {
  uint8_t clock_mode = EEPROM.read(CFG_CLOCK_MODE);

  if (clock_mode > 1) {
    clock_mode = 1;
    __cfg_set_clock_mode(cfg, clock_mode);
  }

  cfg->clock_mode = clock_mode;
  ios_clk_set_prescale(clock_mode ? 2 : 1);
}

void ios_cfg_setup(struct ios_cfg *cfg) {
  __cfg_nr_boot_mode_setup(cfg);
  __cfg_boot_mode_setup(cfg);
  __cfg_autoexec_en_setup(cfg);
  __cfg_clock_mode_setup(cfg);
  __cfg_load_from_csv(cfg);
}
