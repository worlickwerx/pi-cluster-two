/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "src/libbramble/bramble.h"

int firmware_version_main (int argc, char *argv[])
{
    int fd;
    char version[NVRAM_VERSION_SIZE];

    if (argc != 1)
        die ("Usage: bramble firmware-version\n");

    if ((fd = nvram_open (O_RDONLY)) < 0)
        die ("could not open nvram: %s\n", strerror (errno));

    if (nvram_read (fd,
                    NVRAM_VERSION_ADDR,
                    version,
                    sizeof (version)) < 0)
        die ("failed to read version: %s\n", strerror (errno));
    // N.B. result should be null terminated but take no chances
    printf ("%.*s\n", (int)sizeof (version), version);

    close (fd);
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

