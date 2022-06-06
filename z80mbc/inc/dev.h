#ifndef __DEV_H__
#define __DEV_H__

typedef int ssize_t;

#include "ios.h"

#include "dev_bank.h"
#include "dev_gpio.h"
#include "dev_rtc.h"
#include "dev_sd.h"
#include "dev_tty.h"
#include "dev_user.h"

extern void dev_install(struct ios *ios);

#endif /* __DEV_H__ */
