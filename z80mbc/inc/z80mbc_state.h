#ifndef __Z80MBC_STATE_H__
#define __Z80MBC_STATE_H__

#include "ios.h"

typedef void (*z80mbc_state_t)(struct ios *ios);

extern z80mbc_state_t z80mbc_state_machine;

extern void z80mbc_state_set(z80mbc_state_t new_state);

#endif /* __Z80MBC_STATE_H__ */
