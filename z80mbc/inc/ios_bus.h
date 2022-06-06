#ifndef __IOS_BUS_H__
#define __IOS_BUS_H__

extern void ios_bus_set_nBUSREQ_HIGH(void);
extern void ios_bus_set_nBUSREQ_LOW(void);
extern uint8_t ios_bus_get_A0(void);
extern void ios_bus_set_RAM_CE2_HIGH(void);
extern void ios_bus_set_RAM_CE2_LOW(void);
extern uint8_t ios_bus_read(void);
extern void ios_bus_write(uint8_t data);
extern void ios_bus_release(void);
extern void ios_bus_setup(void);

#endif /* __IOS_BUS_H__ */
