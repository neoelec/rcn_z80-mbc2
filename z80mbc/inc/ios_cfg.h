#ifndef __IOS_CFG_H__
#define __IOS_CFG_H__

struct ios_cfg {
  uint8_t nr_boot_mode;
  uint8_t boot_mode;
  uint8_t autoexec_en;
  uint8_t clock_mode;
  char boot_name[24];
  int8_t idx_built_in;
  char boot_file[13];
  uint16_t base_addr;
  uint16_t boot_addr;
  uint16_t boot_size;
  int8_t disk_set;
  uint8_t use_irq_tty_rx;
};

extern void ios_cfg_print_boot_mode(struct ios_cfg *cfg);
extern uint8_t ios_cfg_get_nr_boot_mode(struct ios_cfg *cfg);
extern uint8_t ios_cfg_get_boot_mode(struct ios_cfg *cfg);
extern void ios_cfg_set_boot_mode(struct ios_cfg *cfg, uint8_t boot_mode);
extern uint8_t ios_cfg_get_autoexec_en(struct ios_cfg *cfg);
extern void ios_cfg_set_autoexec_en(struct ios_cfg *cfg, uint8_t autoexec_en);
extern uint8_t ios_cfg_get_clock_mode(struct ios_cfg *cfg);
extern void ios_cfg_set_clock_mode(struct ios_cfg *cfg, uint8_t clock_mode);
extern void ios_cfg_print_cfg(const struct ios_cfg *cfg);
extern void ios_cfg_load_from_csv(struct ios_cfg *cfg, uint8_t mode);
extern void ios_cfg_setup(struct ios_cfg *cfg);

#endif /* __IOS_CFG_H__ */
