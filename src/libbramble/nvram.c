/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

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

#include "nvram.h"

int nvram_open (int flags)
{
    int fd;

    fd = open (BRAMBLE_NVRAM_PATH, flags);
    if (fd < 0)
        return -1;
    return fd;
}

int nvram_read (int fd, int reg, void *data, size_t len)
{
    ssize_t n;

    n = pread (fd, data, len, reg);
    if (n < 0)
        return -1;
    if (n < len) {
        errno = EAGAIN;
        return -1;
    }
    return 0;
}

int nvram_write (int fd, int reg, const void *data, size_t len)
{
    ssize_t n;

    n = pwrite (fd, data, len, reg);
    if (n < 0)
        return -1;
    if (n < len) {
        errno = EAGAIN;
        return -1;
    }
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
