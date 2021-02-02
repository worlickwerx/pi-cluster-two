/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include "src/libbramble/bramble.h"
#include "utils.h"

void die (const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
    exit (1);
}

void warn (const char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
}


double monotime (void)
{
    struct timespec ts;

    clock_gettime (CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1E-9;
}

double monotime_since (double t)
{
    return monotime () - t;
}

int slot_parse (const char *s)
{
    char *endptr;
    int slot;

    errno = 0;
    slot = strtol (s, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || slot < 0 || slot > 15)
        return -1;
    return slot;
}

int slot_get (void)
{
    int fd;
    uint8_t slot;

    fd = i2c_open (BRAMBLE_I2C_DEVICE, I2C_ADDRESS);
    if (fd < 0)
        return -1;
    if (i2c_read (fd, I2C_REG_SLOT, &slot, 1) < 0) {
        close (fd);
        return -1;
    }
    close (fd);
    return slot;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

