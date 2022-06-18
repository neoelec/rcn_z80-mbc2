// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"
#include "inc/ios.h"
#include "inc/z80mbc.h"

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
  struct ios *ios = ios_get_instance();
  struct ios_cfg *cfg = &ios->cfg;
  String str_bm;
  int nr_boot_mode = ios_cfg_get_nr_boot_mode(cfg);
  long boot_mode;

  Serial.println();
  Serial.printf(F("MBC: BOOT MODE [0-%d]"), nr_boot_mode - 1);
  if (!menu_cmd.getStrValue(str_bm) || !str_bm.length()) {
    boot_mode = ios_cfg_get_boot_mode(cfg);
    goto __exit;
  }

  boot_mode = str_bm.toInt();

  if (boot_mode < 0 || boot_mode >= nr_boot_mode) {
    Serial.println();
    Serial.printf(F("Wrong Value %d. It should be 0 <= boot_mode <= %d"),
        boot_mode, nr_boot_mode - 1);
    Serial.println();
    goto __no_change;
  }

__exit:
  Serial.println();
  ios_cfg_set_boot_mode(cfg, static_cast<uint8_t>(boot_mode & 0xFF));
__no_change:
  menu_cmd.giveCmdPrompt();
}

static void ____menu_adjust_rtc(struct dev_rtc *rtc, stDateTimeGroup &dtg) {
  DateTime dt(dtg.u16Year, dtg.u8Month, dtg.u8Day, dtg.u8Hour, dtg.u8Min, dtg.u8Sec);
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

  ____menu_adjust_rtc(rtc, dtg);

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

static void __menu_setup(void) {
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
    { txt_x, 'x',[](){ Serial.println(); z80mbc_state_set(z80mbc_state_bl_run); }},
    { txt__, '?',[](){ menu_cmd.ShowMenu(); menu_cmd.giveCmdPrompt(); }}
  };

  if (!menu_cmd.begin(menu_list, ARRAY_SIZE(menu_list), prompt)) {
    Serial.println(F("MBC: MENU Failed"));
    while (1) ;
  }

  menu_cmd.ShowMenu();
  menu_cmd.giveCmdPrompt();
}

void z80mbc_state_menu_load(struct ios *ios) {
  Serial.println(F("MBC: Boot-Menu"));

  ios_cfg_print_cfg(&ios->cfg);
  __menu_setup();

  z80mbc_state_set(z80mbc_state_menu_run);
}

void z80mbc_state_menu_run(struct ios *ios) {
  uint8_t cmd;

  z80mbc_blink_ios_led();

  cmd = menu_cmd.UserRequest();
  if (cmd)
    menu_cmd.ExeCommand(cmd);
}
