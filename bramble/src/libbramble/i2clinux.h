/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _BRAMBLE_I2CLINUX_H
#define _BRAMBLE_I2CLINUX_H

#define BRAMBLE_I2C_DEVICE                  "/dev/i2c-1"

int i2c_open (const char *name, int address);
int i2c_read (int fd, int reg, void *data, size_t len);
int i2c_write (int fd, int reg, const void *data, size_t len);

#endif /* !_BRAMBLE_I2CLINUX_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
