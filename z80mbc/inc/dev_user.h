#ifndef __DEV_USER_H__
#define __DEV_USER_H__

struct dev_user {
  struct e8bit_io_dev io_dev;
  struct ios *ios;
  uint8_t led_status;
};

extern uint8_t dev_user_get_boot_selection(struct dev_user *user);
extern struct dev_user *dev_user_get_instance(void);
extern void dev_user_install(struct ios *ios);

#endif /* __DEV_USER_H__ */
