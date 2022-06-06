#ifndef __DEV_SD_H__
#define __DEV_SD_H__

#include <pff.h>
#include <pffArduino.h>

#if (SD_CS_PIN!=4)
#error "SD_CS_PIN should be 4 in pffArduino.h"
#endif

#if (_USE_DIR!=0)
#error "_USE_DIR should be 0 in pffconf.h"
#endif

#if (_FS_FAT12!=0)
#error "_FS_FAT12 should be 0 in pffconf.h"
#endif

#define DEV_SD_BUF_SZ     32

enum {
  E_DEV_SD_MOUNT = 0,
  E_DEV_SD_OPEN,
  E_DEV_SD_READ,
  E_DEV_SD_WRITE,
  E_DEV_SD_SEEK,
};

enum {
  E_DISK_ILL_DISK_NUM = 16,
  E_DISK_ILL_TRACK_NUM,
  E_DISK_ILL_SECT_NUM,
  E_DISK_UNEXPECTED_EOF,
};

struct dev_sd {
  struct e8bit_io_dev io_dev;
  struct ios *ios;
  FATFS fatfs;
  const char *filename;
  uint8_t buf[DEV_SD_BUF_SZ];
  uint16_t track_sel;
  uint8_t sect_sel;
  uint8_t disk_err;
  char disk_name[13];
};

extern ssize_t dev_sd_open_raw(struct dev_sd *sd, const char *filename);
extern ssize_t dev_sd_read_raw(struct dev_sd *sd, void *buf, size_t sz_to_read);
extern ssize_t dev_sd_write_raw(struct dev_sd *sd, const void *buf, size_t sz_to_write);
extern void dev_sd_hang_on_err(ssize_t err, uint8_t operation, const char *filename);
extern struct dev_sd *dev_sd_get_instance(void);
extern void dev_sd_install(struct ios *ios);

#endif /* __DEV_SD_H__ */
