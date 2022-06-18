// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"
#include "inc/z80mbc.h"

static void __rtc_print_date_time(DateTime &date_time) {
  Serial.printf(F("%02u/%02u/%04u %02u:%02u:%02u"),
      date_time.day(), date_time.month(), date_time.year(),
      date_time.hour(), date_time.minute(), date_time.second());
}

void dev_rtc_print_current_date_time(struct dev_rtc *rtc) {
  RTC_DS3231 &ds3231 = rtc->ds3231;
  DateTime &date_time = rtc->date_time;

  date_time = ds3231.now();

  Serial.print(F("DEV: DS2313 RTC Date/Time - "));
  __rtc_print_date_time(date_time);
  Serial.println();
}

bool dev_rtc_is_avaiable(struct dev_rtc *rtc) {
  return rtc->is_available;
}

static void __rtc_update(struct dev_rtc *rtc, DateTime *dt) {
  RTC_DS3231 &ds3231 = rtc->ds3231;

  ds3231.adjust(*dt);
}

void dev_rtc_update(struct dev_rtc *rtc, DateTime *dt) {
  if (!rtc->is_available)
    return;

  __rtc_update(rtc, dt);
}

static inline void __rtc_read(struct dev_rtc *rtc, uint8_t *mem, uint16_t addr) {
  struct ios *ios = rtc->ios;
  RTC_DS3231 &ds3231 = rtc->ds3231;
  DateTime &date_time = rtc->date_time;

  switch (ios->io_count++) {
    case E_DEV_RTC_SECOND:
      date_time = ds3231.now();
      ios->io_data = date_time.second();
      break;
    case E_DEV_RTC_MINUTE:
      ios->io_data = date_time.minute();
      break;
    case E_DEV_RTC_HOUR:
      ios->io_data = date_time.hour();
      break;
    case E_DEV_RTC_DAY:
      ios->io_data = date_time.day();
      break;
    case E_DEV_RTC_MONTH:
      ios->io_data = date_time.month();
      break;
    case E_DEV_RTC_YEAR:
      ios->io_data = static_cast<uint8_t>(date_time.year() - 2000);
      break;
    case E_DEV_RTC_TEMPERATURE:
      ios->io_data = static_cast<uint8_t>(ds3231.getTemperature());
      break;
    default:
      ios->io_command == E_IOS_NO_OPERATION;
      break;
  }

  ios->io_handled = true;
}

static void dev_rtc_read(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct dev_rtc *rtc = container_of(io_dev, struct dev_rtc, io_dev);
  struct ios *ios = rtc->ios;

  if (ios->io_command != E_IOS_RD_DATETIME)
    return;

  if (!rtc->is_available) {
    ios->io_command = E_IOS_NO_OPERATION;
    return;
  }

  __rtc_read(rtc, mem, addr);
}

static inline void __rtc_init_on_available(struct dev_rtc *rtc) {
  struct e8bit_io_dev *io_dev = &rtc->io_dev;
  RTC_DS3231 &ds3231 = rtc->ds3231;

  rtc->is_available = true;

  if (ds3231.lostPower()) {
    Serial.println(F("DEV: DS2313 RTC Date/Time - Failure! Compiled Date/Time is applied."));
    ds3231.adjust(rtc->compiled_date_time);
  }

  dev_rtc_print_current_date_time(rtc);

  io_dev->read = dev_rtc_read;
}

static inline void __rtc_init_on_unavailable(struct dev_rtc *rtc) {
  DateTime &date_time = rtc->date_time;

  rtc->is_available = false;
  date_time = rtc->compiled_date_time;

  Serial.println(F("DEV: DS2313 RTC Date/Time - Not Found."));
}

static struct dev_rtc *__rtc_get_instance(void) {
  static struct dev_rtc __rtc;
  struct dev_rtc *rtc = &__rtc;
  RTC_DS3231 &ds3231 = rtc->ds3231;

  rtc->compiled_date_time = DateTime(__DATE__, __TIME__);

  Serial.print(F("DEV: Compiled Date/Time   - "));
  __rtc_print_date_time(rtc->compiled_date_time);
  Serial.println();

  if (ds3231.begin())
    __rtc_init_on_available(rtc);
  else
    __rtc_init_on_unavailable(rtc);

  return rtc;
}

struct dev_rtc *dev_rtc_get_instance(void) {
  static struct dev_rtc *rtc;

  if (rtc)
    return rtc;

  rtc = __rtc_get_instance();

  return rtc;
}

void dev_rtc_install(struct ios *ios) {
  struct dev_rtc *rtc = dev_rtc_get_instance();

  rtc->ios = ios;

  ios_flag_rtc_en_set(&ios->flag, rtc->is_available);

  e8bit_io_attach_dev(&ios->io, &rtc->io_dev);
}
