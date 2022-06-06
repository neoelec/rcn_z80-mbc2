#ifndef __IOS_CLK_H__
#define __IOS_CLK_H__

extern void ios_clk_set_prescale(uint16_t scale);
extern void ios_clk_enable(void);
extern void ios_clk_disable(void);
extern void ios_clk_pulse(size_t n);
extern void ios_clk_setup(void);

#endif /* __IOS_CLK_H__ */
