// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"

static inline void __gpio_read(struct dev_gpio *gpio, uint8_t *mem, uint16_t addr) {
  struct ios *ios = gpio->ios;
  Adafruit_MCP23X17 &mcp = gpio->mcp;

  switch (ios->io_command) {
    case E_IOS_RD_GPIOA:
      ios->io_data = mcp.readGPIOA();
      break;
    case E_IOS_RD_GPIOB:
      ios->io_data = mcp.readGPIOB();
      break;
    default:
      return;
  }

  ios->io_command = E_IOS_NO_OPERATION;
  ios->io_handled = true;
}

static void dev_gpio_read(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct dev_gpio *gpio = container_of(io_dev, struct dev_gpio, io_dev);
  struct ios *ios = gpio->ios;

  if (ios->io_command < E_IOS_RD_GPIOA || ios->io_command > E_IOS_RD_GPIOB)
    return;

  if (gpio->is_avaiable)
    __gpio_read(gpio, mem, addr);
}

static void __gpio_write_IODIRA(struct dev_gpio *gpio, uint8_t data) {
  Adafruit_MCP23X17 &mcp = gpio->mcp;
  uint8_t pin;

  for (pin = 0; pin < 8; pin++) {
    uint8_t mode = data & (1 << pin) ? INPUT : OUTPUT;
    mcp.pinMode(pin, mode);
  }
}

static void __gpio_write_IODIRB(struct dev_gpio *gpio, uint8_t data) {
  Adafruit_MCP23X17 &mcp = gpio->mcp;
  uint8_t pin;

  for (pin = 0; pin < 8; pin++) {
    uint8_t mode = data & (1 << pin) ? INPUT : OUTPUT;
    mcp.pinMode(pin + 8, mode);
  }
}

static void __gpio_write_GPPUA(struct dev_gpio *gpio, uint8_t data) {
  Adafruit_MCP23X17 &mcp = gpio->mcp;
  uint8_t pin;

  for (pin = 0; pin < 8; pin++) {
    uint8_t mode = data & (1 << pin) ? INPUT_PULLUP : INPUT;
    mcp.pinMode(pin, mode);
  }
}

static void __gpio_write_GPPUB(struct dev_gpio *gpio, uint8_t data) {
  Adafruit_MCP23X17 &mcp = gpio->mcp;
  uint8_t pin;

  for (pin = 0; pin < 8; pin++) {
    uint8_t mode = data & (1 << pin) ? INPUT_PULLUP : INPUT;
    mcp.pinMode(pin + 8, mode);
  }
}

static inline void __gpio_write(struct dev_gpio *gpio, uint8_t *mem, uint16_t addr) {
  struct ios *ios = gpio->ios;
  Adafruit_MCP23X17 &mcp = gpio->mcp;

  switch (ios->io_command) {
    case E_IOS_WR_GPIOA:
      mcp.writeGPIOA(ios->io_data);
      break;
    case E_IOS_WR_GPIOB:
      mcp.writeGPIOB(ios->io_data);
      break;
    case E_IOS_WR_IODIRA:
      __gpio_write_IODIRA(gpio, ios->io_data);
      break;
    case E_IOS_WR_IODIRB:
      __gpio_write_IODIRB(gpio, ios->io_data);
      break;
    case E_IOS_WR_GPPUA:
      __gpio_write_GPPUA(gpio, ios->io_data);
      break;
    case E_IOS_WR_GPPUB:
      __gpio_write_GPPUB(gpio, ios->io_data);
      break;
    default:
      return;
  }

  ios->io_command = E_IOS_NO_OPERATION;
  ios->io_handled = true;
}

static void dev_gpio_write(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct dev_gpio *gpio = container_of(io_dev, struct dev_gpio, io_dev);
  struct ios *ios = gpio->ios;

  if (ios->io_command < E_IOS_WR_GPIOA || ios->io_command > E_IOS_WR_GPPUB)
    return;

  if (gpio->is_avaiable)
    __gpio_write(gpio, mem, addr);
}

static inline void __gpio_init_on_available(struct dev_gpio *gpio) {
  struct e8bit_io_dev *io_dev = &gpio->io_dev;

  Serial.println(F("DEV: MCP23017 GPIO Expander - Found."));

  gpio->is_avaiable = true;

  io_dev->read = dev_gpio_read;
  io_dev->write = dev_gpio_write;
}

static inline void __gpio_init_on_unavailable(struct dev_gpio *gpio) {
  Serial.println(F("DEV: MCP23017 GPIO Expander - Not Found."));

  gpio->is_avaiable = false;
}

static struct dev_gpio *__gpio_get_instance(void) {
  static struct dev_gpio __gpio;
  struct dev_gpio *gpio = &__gpio;
  Adafruit_MCP23X17 &mcp = gpio->mcp;

  if (mcp.begin_I2C())
    __gpio_init_on_available(gpio);
  else
    __gpio_init_on_unavailable(gpio);

  return gpio;
}

struct dev_gpio *dev_gpio_get_instance(void) {
  static struct dev_gpio *gpio;

  if (gpio)
    return gpio;

  gpio = __gpio_get_instance();

  return gpio;
}

void dev_gpio_install(struct ios *ios) {
  struct dev_gpio *user = dev_gpio_get_instance();

  user->ios = ios;

  e8bit_io_attach_dev(&ios->io, &user->io_dev);
}
