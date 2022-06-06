#ifndef __DEV_GPIO_H__
#define __DEV_GPIO_H__

#include <Adafruit_MCP23X17.h>

struct dev_gpio {
  struct e8bit_io_dev io_dev;
  struct ios *ios;
  bool is_avaiable;
  Adafruit_MCP23X17 mcp;
};

extern struct dev_gpio *dev_gpio_get_instance(void);
extern void dev_gpio_install(struct ios *ios);

#endif /* __DEV_GPIO_H__ */
