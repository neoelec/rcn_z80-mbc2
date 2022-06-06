#ifndef __COMPILED_DATE_TIME_H__
#define __COMPILED_DATE_TIME_H__

#include <RTClib.h>

class CompiledDateTime {
public:
  CompiledDateTime() {
    char date_time[] = __DATE__ " " __TIME__;
    const char *month_name[] = {
      NULL,
      "Jan", "Feb", "Mar", "Apr", "May", "Jun",
      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    /*
       0         1
       01234567890123456789
       May 25 2022 23:18:47
     */
    date_time[3] = '\0';
    date_time[6] = '\0';
    date_time[11] = '\0';
    date_time[14] = '\0';
    date_time[17] = '\0';

    year = static_cast<uint16_t>(atoi(&date_time[7]));
    day = static_cast<uint8_t>(atoi(&date_time[4]));
    hour = static_cast<uint8_t>(atoi(&date_time[12]));
    minute = static_cast<uint8_t>(atoi(&date_time[15]));
    second = static_cast<uint8_t>(atoi(&date_time[18]));

    for (uint8_t i = 1; i < ARRAY_SIZE(month_name); i++) {
      if (!strncmp(date_time, month_name[i], 3)) {
        month = i;
        break;
      }
    }
  }

  DateTime dateTime(void) {
    return DateTime(year, month, day, hour, minute, second);
  }

private:
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

#endif /* __COMPILE_TIME_H__ */
