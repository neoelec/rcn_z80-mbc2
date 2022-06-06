#ifndef __IOS_TTY_H__
#define __IOS_TTY_H__

struct dev_tty {
  struct e8bit_io_dev io_dev;
  struct ios *ios;
};

extern struct dev_tty *dev_tty_get_instance(void);
extern void dev_tty_install(struct ios *ios);

#endif /* __IOS_TTY_H__ */
