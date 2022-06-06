#ifndef __DEV_BANK_H__
#define __DEV_BANK_H__

struct dev_bank {
  struct e8bit_io_dev io_dev;
  struct ios *ios;
  uint8_t num;
};

extern struct dev_bank *dev_bank_get_instance(void);
extern void dev_bank_install(struct ios *ios);

#endif /* __DEV_BANK_H__ */
