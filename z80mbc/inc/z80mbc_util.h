#ifndef __Z80MBC_UTIL_H__
#define __Z80MBC_UTIL_H__

extern void z80mbc_putstr_P(const char *str);
extern void z80mbc_putstrln_P(const char *str);
extern void z80mbc_printf_P(const char *fmt, ...);
extern void z80mbc_blink_ios_led(void);
extern void z80mbc_util_setup(void);

#endif /* __Z80MBC_UTIL_H__ */
