#ifndef __DEV_RTC_H__
#define __DEV_RTC_H__

#include <RTClib.h>

enum {
  E_DEV_RTC_SECOND = 0,
  E_DEV_RTC_MINUTE,
  E_DEV_RTC_HOUR,
  E_DEV_RTC_DAY,
  E_DEV_RTC_MONTH,
  E_DEV_RTC_YEAR,
  E_DEV_RTC_TEMPERATURE,
  E_DEV_RTC_MAX = E_DEV_RTC_TEMPERATURE,
};

struct dev_rtc {
  struct e8bit_io_dev io_dev;
  struct ios *ios;
  bool is_available;
  RTC_DS3231 ds3231;
  DateTime date_time;
  DateTime compiled_date_time;
};

extern void dev_rtc_print_current_date_time(struct dev_rtc *rtc);
extern bool dev_rtc_is_avaiable(struct dev_rtc *rtc);
extern void dev_rtc_update(struct dev_rtc *rtc, DateTime *dt);
extern struct dev_rtc *dev_rtc_get_instance(void);
extern void dev_rtc_install(struct ios *ios);

#endif /* __DEV_RTC_H__ */
