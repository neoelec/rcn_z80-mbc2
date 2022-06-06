#ifndef __IOS_CPU_H__
#define __IOS_CPU_H__

extern uint8_t ios_cpu_get_nWAIT(void);
extern void ios_cpu_set_nWAIT_HIGH(void);
extern void ios_cpu_set_nWAIT_LOW(void);
extern void ios_cpu_set_nINT_HIGH(void);
extern void ios_cpu_set_nINT_LOW(void);
extern void ios_cpu_set_nRESET_HIGH(void);
extern void ios_cpu_set_nRESET_LOW(void);
extern void ios_cpu_setup(void);

#endif /* __IOS_CPU_H__ */
