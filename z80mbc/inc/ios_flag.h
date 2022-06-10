#ifndef __IOS_FLAG_H__
#define __IOS_FLAG_H__

struct ios_flag {
  e8bit_io_dev io_dev;
  union {
    uint8_t io_data;
    struct {
      uint8_t autoexec_en:1;
      uint8_t rtc_en:1;
      uint8_t rx_available:1;
      uint8_t rx_last_empty:1;
      uint8_t reserved_0:4;
    };
  };
};

extern void ios_flag_autoexec_en_set(struct ios_flag *flag, uint8_t autoexec_en);
extern void ios_flag_rtc_en_set(struct ios_flag *flag, uint8_t rtc_en);
extern void ios_flag_tty_rx_last_empty_set(struct ios_flag *flag, uint8_t rx_last_empty);
extern void ios_flag_setup(struct ios_flag *flag);

#endif /* __IOS_FLAG_H__ */
