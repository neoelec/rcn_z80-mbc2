#ifndef __Z80MBC_BL_H__
#define __Z80MBC_BL_H__

#include "ios.h"

extern void z80mbc_bl_load(struct ios *ios);
extern void z80mbc_bl_run(struct ios *ios);
extern void z80mbc_bl_epilog(struct ios *ios);

#endif /* __Z80MBC_BL_H__ */
