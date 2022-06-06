// SPDX-License-Identifier: GPL-3.0-or-later
#include "inc/dev.h"
#include "inc/z80mbc.h"

#define DEV_SD_DISK_MAX   99
#define DEV_SD_DISK_FMT   "DSxNyy.DSK"

static void __sd_print_err_code(ssize_t err) {
  static const char err_fmt[] PROGMEM = "DEV: SD error %u ";

  z80mbc_printf_P(err_fmt, (unsigned int)err);

  switch (err) {
    case FR_OK:
      Serial.print(F("(FR_OK)"));
      break;
    case FR_DISK_ERR:
      Serial.print(F("(FR_DISK_ERR)"));
      break;
    case FR_NOT_READY:
      Serial.print(F("(FR_NOT_READY)"));
      break;
    case FR_NO_FILE:
      Serial.print(F("(FR_NO_FILE)"));
      break;
    case FR_NOT_OPENED:
      Serial.print(F("(FR_NOT_OPENED)"));
      break;
    case FR_NOT_ENABLED:
      Serial.print(F("(FR_NOT_ENABLED)"));
      break;
    case FR_NO_FILESYSTEM:
      Serial.print(F("(FR_NO_FILESYSTEM)"));
      break;
    default:
      Serial.print(F("(UNKNOWN)"));
      break;
  }
}

static void __sd_print_err_operation(uint8_t operation) {
  Serial.print(F(" on "));

  switch (operation) {
    case E_DEV_SD_MOUNT:
      Serial.print(F("MOUNT"));
      break;
    case E_DEV_SD_OPEN:
      Serial.print(F("OPEN"));
      break;
    case E_DEV_SD_READ:
      Serial.print(F("READ"));
      break;
    case E_DEV_SD_WRITE:
      Serial.print(F("WRITE"));
      break;
    case E_DEV_SD_SEEK:
      Serial.print(F("SD_SEEK"));
      break;
    default:
      Serial.print(F("UNKNOWN"));
      break;
  }
}

static void __sd_print_err_filename(const char *filename) {
  if (filename) {
    static const char fname_fmt[] PROGMEM = " - File : %s\n";
    z80mbc_printf_P(fname_fmt, filename);
  } else
    Serial.println();
}

static void __sd_print_err(ssize_t err, uint8_t operation, const char *filename) {
  __sd_print_err_code(err);
  __sd_print_err_operation(operation);
  __sd_print_err_filename(filename);
}

void dev_sd_hang_on_err(ssize_t err, uint8_t operation, const char *filename) {
  if (err > 0)
    err = -err;

  if (err == FR_OK)
    return;

  __sd_print_err(err, operation, filename);
  while (1) ;
}

static void __sd_wait_key(void) {
  while (Serial.available() > 0)
    Serial.read();

  Serial.println(F("DEV: Check SD and press a key to repeat"));
  Serial.println();

  while (Serial.available() < 1) ;
}

ssize_t dev_sd_open_raw(struct dev_sd *sd, const char *filename) {
  FRESULT err;

  err = PF.open(filename);
  if (err != FR_OK)
    return -static_cast<ssize_t>(err);

  sd->filename = filename;

  return 0;
}

ssize_t dev_sd_read_raw(struct dev_sd *sd, void *buf, size_t __sz_to_read) {
  FRESULT err;
  UINT sz_to_read = static_cast<UINT>(__sz_to_read);
  UINT sz_read;

  err = PF.readFile(buf, sz_to_read, &sz_read);
  if (err != FR_OK)
    return -static_cast<ssize_t>(err);

  return static_cast<ssize_t>(sz_read);
}

ssize_t dev_sd_write_raw(struct dev_sd *sd, const void *buf, size_t __sz_to_write) {
  FRESULT err;
  UINT sz_to_write = static_cast<UINT>(__sz_to_write);
  UINT sz_wrote;

  err = PF.writeFile(buf, sz_to_write, &sz_wrote);
  if (err != FR_OK)
    return -static_cast<ssize_t>(err);

  return static_cast<ssize_t>(sz_wrote);
}

ssize_t dev_sd_lseek_raw(struct dev_sd *sd, size_t sect_num) {
  FRESULT err;
  DWORD ofs = static_cast<DWORD>(sect_num) << 9;

  err = PF.seek(ofs);
  if (err != FR_OK)
    return -static_cast<ssize_t>(err);

  return 0;
}

static void __sd_read_ERRDISK(struct dev_sd *sd, struct ios *ios) {
  ssize_t err;

  ios->io_data = sd->disk_err;
}

static inline void __sd_read_READSECT_lseek(struct dev_sd *sd) {
  if (sd->disk_err)
    return;

  if ((sd->track_sel < 512) && (sd->sect_sel < 32)) {
    ssize_t err;

    sd->disk_err = 0;

    err = dev_sd_lseek_raw(sd, (sd->track_sel << 5) | sd->sect_sel);
    if (err < 0)
      sd->disk_err = static_cast<uint8_t>((-err) & 0xFF);
  }
}

static inline void __sd_read_READSECT_read(struct dev_sd *sd, struct ios *ios, size_t sz_to_read) {
  ssize_t sz_read;

  if (sd->disk_err)
    return;

  if (!sz_to_read) {
    sd->disk_err = 0;

    sz_read = dev_sd_read_raw(sd, sd->buf, DEV_SD_BUF_SZ);
    if (sz_read < 0)
      sd->disk_err = static_cast<uint8_t>((-sz_read) & 0xFF);
    else if (sz_read < DEV_SD_BUF_SZ)
      sd->disk_err = E_DISK_UNEXPECTED_EOF;
  }
}

static inline void __sd_read_READSECT_transfer(struct dev_sd *sd,
    struct ios *ios, size_t sz_to_read) {
  if (sd->disk_err)
    return;

  ios->io_data = sd->buf[sz_to_read];
}

static void __sd_read_READSECT(struct dev_sd *sd, struct ios *ios) {
  size_t sz_to_read;
  ssize_t sz_read;

  if (!ios->io_count)
    __sd_read_READSECT_lseek(sd);

  sz_to_read = ios->io_count % DEV_SD_BUF_SZ;
  __sd_read_READSECT_read(sd, ios, sz_to_read);
  __sd_read_READSECT_transfer(sd, ios, sz_to_read);

  if (ios->io_count >= 511) {
    ios->io_command = E_IOS_NO_OPERATION;
  }

  ios->io_count++;
}

static void __sd_read_SDMOUNT(struct dev_sd *sd, struct ios *ios) {
  FRESULT err = PF.begin(&sd->fatfs);

  ios->io_data = static_cast<uint8_t>(err & 0xFF);
}

static void dev_sd_read(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct dev_sd *sd = container_of(io_dev, struct dev_sd, io_dev);
  struct ios *ios = sd->ios;

  switch (ios->io_command) {
    case E_IOS_RD_READSECT:
      __sd_read_READSECT(sd, ios);
      break;
    case E_IOS_RD_ERRDISK:
      __sd_read_ERRDISK(sd, ios);
      break;
    case E_IOS_RD_SDMOUNT:
      __sd_read_SDMOUNT(sd, ios);
      break;
    default:
      return;
  }

  if (ios->io_command != E_IOS_RD_READSECT)
    ios->io_command == E_IOS_NO_OPERATION;

  ios->io_handled = true;
}

static void __sd_write_SELDISK(struct dev_sd *sd, struct ios *ios) {
  struct ios_cfg *cfg = &ios->cfg;
  ssize_t err;

  if (ios->io_data > DEV_SD_DISK_MAX) {
    sd->disk_err = E_DISK_ILL_DISK_NUM;
    return;
  }

  sd->disk_name[2] = cfg->disk_set + '0';
  sd->disk_name[4] = (ios->io_data / 10) + '0';
  sd->disk_name[5] = ios->io_data - ((ios->io_data / 10) * 10) + '0';

  sd->disk_err = 0;

  err = dev_sd_open_raw(sd, sd->disk_name);
  if (err < 0)
    sd->disk_err = static_cast<uint8_t>((-err) & 0xFF);
}

static void __sd_write_SELTRACK(struct dev_sd *sd, struct ios *ios) {
  if (!ios->io_count) {
    sd->track_sel = ios->io_data;
    goto __exit_lsb;
  }

  sd->disk_err = 0;
  sd->track_sel = (static_cast<uint16_t>(ios->io_data) << 8) | lowByte(sd->track_sel);

  if (sd->track_sel >= 512)
    sd->disk_err = E_DISK_ILL_TRACK_NUM;

  if (sd->track_sel >= 32)
    sd->disk_err = E_DISK_ILL_SECT_NUM;

  ios->io_command = E_IOS_NO_OPERATION;

__exit_lsb:
  ios->io_count++;
}

static void __sd_write_SELSECT(struct dev_sd *sd, struct ios *ios) {
  sd->disk_err = 0;
  sd->sect_sel = ios->io_data;

  if (sd->track_sel >= 512)
    sd->disk_err = E_DISK_ILL_TRACK_NUM;

  if (sd->sect_sel >= 32)
    sd->disk_err = E_DISK_ILL_SECT_NUM;
}

static inline void __sd_write_WRITESECT_lseek(struct dev_sd *sd) {
  __sd_read_READSECT_lseek(sd);
}

static inline void __sd_write_WRITESECT_write(struct dev_sd *sd,
    struct ios *ios, size_t sz_to_write) {
  sd->buf[sz_to_write] = ios->io_data;
}

static inline void __sd_write_WRITESECT_sync(struct dev_sd *sd, struct ios *ios, size_t sz_to_write) {
  ssize_t sz_wrote;

  if (sz_to_write != (DEV_SD_BUF_SZ - 1))
    return;

  sd->disk_err = 0;

  sz_wrote = dev_sd_write_raw(sd, sd->buf, DEV_SD_BUF_SZ);
  if (sz_wrote < 0)
    sd->disk_err = static_cast<uint8_t>((-sz_wrote) & 0xFF);
  else if (sz_wrote < DEV_SD_BUF_SZ)
    sd->disk_err = E_DISK_UNEXPECTED_EOF;

  if (ios->io_count >= 511) {
    if (!sd->disk_err) {
      sd->disk_err = 0;
      sz_wrote = dev_sd_write_raw(sd, NULL, 0);
      if (sz_wrote < 0)
        sd->disk_err = static_cast<uint8_t>((-sz_wrote) & 0xFF);
    }

    ios->io_command = E_IOS_NO_OPERATION;
  }
}

static void __sd_write_WRITESECT(struct dev_sd *sd, struct ios *ios) {
  size_t sz_to_write;
  ssize_t sz_wrote;

  if (!ios->io_count)
    __sd_write_WRITESECT_lseek(sd);

  if (!sd->disk_err) {
    sz_to_write = ios->io_count % DEV_SD_BUF_SZ;
    __sd_write_WRITESECT_write(sd, ios, sz_to_write);
    __sd_write_WRITESECT_sync(sd, ios, sz_to_write);
  }

  ios->io_count++;
}

static void dev_sd_write(struct e8bit_io_dev *io_dev, uint8_t *mem, uint16_t addr) {
  struct dev_sd *sd = container_of(io_dev, struct dev_sd, io_dev);
  struct ios *ios = sd->ios;

  switch (ios->io_command) {
    case E_IOS_WR_SELDISK:
      __sd_write_SELDISK(sd, ios);
      break;
    case E_IOS_WR_SELTRACK:
      __sd_write_SELTRACK(sd, ios);
      break;
    case E_IOS_WR_SELSECT:
      __sd_write_SELSECT(sd, ios);
      break;
    case E_IOS_WR_WRITESECT:
      __sd_write_WRITESECT(sd, ios);
      break;
    default:
      return;
  }

  if (ios->io_command != E_IOS_WR_SELTRACK && ios->io_command != E_IOS_WR_WRITESECT)
    ios->io_command = E_IOS_NO_OPERATION;

  ios->io_handled = true;
}

static void __sd_init_unavailable(struct dev_sd *sd, FRESULT err) {
  __sd_print_err(err, E_DEV_SD_MOUNT, NULL);

  if (err == FR_OK)
    return;

  __sd_wait_key();
}

void __sd_init_available(struct dev_sd *sd) {
  struct e8bit_io_dev *io_dev = &sd->io_dev;
  static const char disk_name[] PROGMEM = DEV_SD_DISK_FMT;

  sd->disk_err = E_DISK_UNEXPECTED_EOF;
  strcpy_P(sd->disk_name, disk_name);

  io_dev->read = dev_sd_read;
  io_dev->write = dev_sd_write;
}

static struct dev_sd *__sd_get_instance(void) {
  static struct dev_sd __sd;
  struct dev_sd *sd = &__sd;
  FATFS *fatfs = &sd->fatfs;
  FRESULT err;

  do {
    err = PF.begin(fatfs);
    __sd_init_unavailable(sd, err);
  } while (err != FR_OK);

  __sd_init_available(sd);

  return sd;
}

struct dev_sd *dev_sd_get_instance(void) {
  static struct dev_sd *sd;

  if (sd)
    return sd;

  sd = __sd_get_instance();

  return sd;
}

void dev_sd_install(struct ios *ios) {
  struct dev_sd *user = dev_sd_get_instance();

  user->ios = ios;

  e8bit_io_attach_dev(&ios->io, &user->io_dev);
}
