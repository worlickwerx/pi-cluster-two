/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _BRAMBLE_NVRAM_H
#define _BRAMBLE_NVRAM_H

#define BRAMBLE_NVRAM_PATH          "/sys/bus/nvmem/devices/ds1307_nvram0/nvmem"

#define NVRAM_MATRIX_COLUMNS_ADDR   0
#define NVRAM_MATRIX_COLUMNS_SIZE   5

#define NVRAM_MATRIX_CHAR_ADDR      5
#define NVRAM_MATRIX_CHAR_SIZE      1

#define NVRAM_SLOT_ADDR             6
#define NVRAM_SLOT_SIZE             1

#define NVRAM_VERSION_ADDR          7
#define NVRAM_VERSION_SIZE          22

int nvram_open (int flags);
int nvram_read (int fd, int reg, void *data, size_t len);
int nvram_write (int fd, int reg, const void *data, size_t len);

#endif /* !_BRAMBLE_NVRAM_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
