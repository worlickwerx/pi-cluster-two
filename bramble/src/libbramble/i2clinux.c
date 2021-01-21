/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "i2clinux.h"

int i2c_open (const char *name, int addr)
{
    int fd;

    if ((fd = open (name, O_RDWR)) < 0)
        return -1;
    if (ioctl (fd, I2C_SLAVE, addr) < 0) {
        int saved_errno = errno;
        close (fd);
        errno = saved_errno;
        return -1;
    }
    return fd;
}

int i2c_read (int fd, int reg, void *data, size_t len)
{
    uint8_t buf[1];
    int n;

    buf[0] = reg;
    n = write (fd, buf, 1);
    if (n < 0)
        return -1;
    if (n != 1) {
        errno = EPROTO;
        return -1;
    }
    n = read (fd, data, len);
    if (n < 0)
        return -1;
    if (n != len) {
        errno = EPROTO;
        return -1;
    }
    return 0;
}

int i2c_write (int fd, int reg, const void *data, size_t len)
{
    uint8_t buf[33];
    int n;

    if (len > 32) {
        errno = EINVAL;
        return -1;
    }
    buf[0] = reg;
    memcpy (&buf[1], data, len);
    n = write (fd, buf, len + 1);
    if (n < 0)
        return -1;
    if (n != len + 1) {
        errno = EPROTO;
        return -1;
    }
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
