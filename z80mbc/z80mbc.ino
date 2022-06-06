// SPDX-License-Identifier: GPL-3.0-or-later
#include <SerialMenuCmd.h>

#include "inc/dev.h"
#include "inc/ios.h"
#include "inc/z80mbc.h"

static void __boot_menu_setup(void);

static void z80mbc_load_menu(struct ios *ios);
static void z80mbc_run_menu(struct ios *ios);
static void z80mbc_load_bootstrap(struct ios *ios);
static void z80mbc_run_boostrap(struct ios *ios);
static void z80mbc_power_on(struct ios *ios);
static void z80mbc_run(struct ios *ios);

static void (*z80mbc_state)(struct ios * ios);

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
    z80mbc_state = z80mbc_load_menu;
  else
    z80mbc_state = z80mbc_load_bootstrap;
}

void loop() {
  struct ios *ios = ios_get_instance();

  z80mbc_state(ios);
}

void serialEvent() {
  struct ios *ios = ios_get_instance();

  if (Serial.available() && ios->cfg.use_interrupt)
    ios_cpu_set_nINT_LOW();
}

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

static SerialMenuCmd menu_cmd;

static void __menu_toggle_atuoexec_en(void) {
  struct ios *ios = ios_get_instance();
  struct ios_cfg *cfg = &ios->cfg;

  Serial.println();
  ios_cfg_set_autoexec_en(cfg, !ios_cfg_get_autoexec_en(cfg));
  menu_cmd.giveCmdPrompt();
}

static void __menu_toggle_clock_mode(void) {
  struct ios *ios = ios_get_instance();
  struct ios_cfg *cfg = &ios->cfg;

  Serial.println();
  ios_cfg_set_clock_mode(cfg, !ios_cfg_get_clock_mode(cfg));
  menu_cmd.giveCmdPrompt();
}

static void __menu_change_boot_mode(void) {
  static const char input_fmt[] PROGMEM = "\nMBC: BOOT MODE [0-%d]";
  static const char warn_fmt[] PROGMEM = "\nWrong Value %d. It should be 0 <= boot_mode <= %d\n";
  struct ios *ios = ios_get_instance();
  struct ios_cfg *cfg = &ios->cfg;
  String str_bm;
  int nr_boot_mode = ios_cfg_get_nr_boot_mode(cfg);
  long boot_mode;

  z80mbc_printf_P(input_fmt, nr_boot_mode - 1);
  if (!menu_cmd.getStrValue(str_bm) || !str_bm.length()) {
    boot_mode = ios_cfg_get_boot_mode(cfg);
    goto __exit;
  }

  boot_mode = str_bm.toInt();

  if (boot_mode < 0 || boot_mode >= nr_boot_mode) {
    z80mbc_printf_P(warn_fmt, boot_mode, nr_boot_mode - 1);
    goto __no_change;
  }

__exit:
  Serial.println();
  ios_cfg_set_boot_mode(cfg, static_cast<uint8_t>(boot_mode & 0xFF));
__no_change:
  menu_cmd.giveCmdPrompt();
}

static void ____menu_adjust_rtc(struct dev_rtc *rtc, stDateTimeGroup *dtg) {
  DateTime dt(dtg->u16Year, dtg->u8Month, dtg->u8Day, dtg->u8Hour, dtg->u8Min, dtg->u8Sec);
  dev_rtc_update(rtc, &dt);
}

static void __menu_adjust_rtc(void) {
  struct dev_rtc *rtc = dev_rtc_get_instance();
  String str_dtg;
  stDateTimeGroup dtg;

  if (!dev_rtc_is_avaiable(rtc)) {
    Serial.println();
    Serial.print(F("MBC: RTC is NOT available"));
    goto __no_change;
  }

  Serial.println();
  Serial.print(F("MBC: [YYYY-MM-DDThh:mm:ss]"));
  if (!menu_cmd.getStrOfChar(str_dtg) || !str_dtg.length())
    goto __no_change;

  if (!menu_cmd.ConvStrToDTg(str_dtg, dtg)) {
    Serial.println();
    Serial.print(F("MBC: Malformed input"));
    goto __no_change;
  }

  ____menu_adjust_rtc(rtc, &dtg);

__no_change:
  Serial.println();
  dev_rtc_print_current_date_time(rtc);
  menu_cmd.giveCmdPrompt();
}

static void __menu_list_boot_mode(void) {
  struct ios *ios = ios_get_instance();
  struct ios_cfg *cfg = &ios->cfg;

  Serial.println();

  ios_cfg_print_boot_mode(cfg);

  menu_cmd.giveCmdPrompt();
}

static void __boot_menu_setup(void) {
  static tMenuCmdTxt prompt[] = "";
  static tMenuCmdTxt txt_b[] = "b - Change Boot Mode";
  static tMenuCmdTxt txt_l[] = "l - List Boot Mode";
  static tMenuCmdTxt txt_a[] = "a - Toggle AUTOEXEC";
  static tMenuCmdTxt txt_c[] = "c - Toggle CLK Mode";
  static tMenuCmdTxt txt_t[] = "t - Adjust RTC";
  static tMenuCmdTxt txt_x[] = "x - Exit";
  static tMenuCmdTxt txt__[] = "? - Help";
  static stMenuCmd menu_list[] = {
    { txt_b, 'b', __menu_change_boot_mode },
    { txt_l, 'l', __menu_list_boot_mode },
    { txt_a, 'a', __menu_toggle_atuoexec_en },
    { txt_c, 'c', __menu_toggle_clock_mode },
    { txt_t, 't', __menu_adjust_rtc },
    { txt_x, 'x',[](){ Serial.println(); z80mbc_state = z80mbc_run_boostrap; }},
    { txt__, '?',[](){ menu_cmd.ShowMenu(); menu_cmd.giveCmdPrompt(); }}
  };

  if (!menu_cmd.begin(menu_list, ARRAY_SIZE(menu_list), prompt)) {
    Serial.println(F("MBC: MENU Failed"));
    while (1) ;
  }

  menu_cmd.ShowMenu();
  menu_cmd.giveCmdPrompt();
}

static void z80mbc_load_menu(struct ios *ios) {
  Serial.println(F("MBC: Boot-Menu"));

  ios_cfg_print_cfg(&ios->cfg);
  __boot_menu_setup();

  z80mbc_state = z80mbc_run_menu;
}

static void z80mbc_run_menu(struct ios *ios) {
  uint8_t cmd;

  z80mbc_blink_ios_led();

  cmd = menu_cmd.UserRequest();
  if (cmd)
    menu_cmd.ExeCommand(cmd);
}

static void z80mbc_load_bootstrap(struct ios *ios) {
  ios_cfg_print_cfg(&ios->cfg);
  Serial.println();

  z80mbc_state = z80mbc_run_boostrap;
}

static void z80mbc_run_boostrap(struct ios *ios) {
  struct ios_cfg *cfg = &ios->cfg;
  static const char load_fmt[] PROGMEM = "MBC: Loading %s@0x%04X ...\n";

  __bl_hard_reset();
  ios_cpu_set_nWAIT_HIGH();

  z80mbc_printf_P(load_fmt, cfg->boot_file, cfg->base_addr);
  __bl_load_file_image(cfg);
  Serial.println(F("     ... Done."));

  z80mbc_state = z80mbc_power_on;
}

static void z80mbc_power_on(struct ios *ios) {
  __bl_hard_reset();
  ios_cpu_set_nWAIT_HIGH();
  ios_cpu_set_nRESET_LOW();
  ios_clk_enable();
  delay(1);                     // Just to be sure...
  __bl_flush_rx();
  ios_cpu_set_nRESET_HIGH();

  z80mbc_state = z80mbc_run;
}

static void z80mbc_run(struct ios *ios) {
  ios_loop(ios);
}
