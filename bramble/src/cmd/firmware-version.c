/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "src/libbramble/bramble.h"

int firmware_version_main (int argc, char *argv[])
{
    int fd;
    uint8_t version;

    if (argc != 1)
        die ("Usage: bramble firmware-version\n");

    if ((fd = i2c_open (BRAMBLE_I2C_DEVICE, I2C_ADDRESS)) < 0)
        die ("%s: %s\n", BRAMBLE_I2C_DEVICE, strerror (errno));

    if (i2c_read (fd, I2C_REG_VERSION, &version, 1) < 0)
        die ("i2c read: %s\n", strerror (errno));

    printf ("%d\n", version);

    close (fd);
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

