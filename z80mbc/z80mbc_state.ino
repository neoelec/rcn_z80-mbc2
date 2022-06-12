// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/z80mbc.h"

z80mbc_state_t z80mbc_state_machine;

void z80mbc_state_set(z80mbc_state_t new_state) {
  z80mbc_state_machine = new_state;
}
