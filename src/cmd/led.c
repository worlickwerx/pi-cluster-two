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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include "src/libbramble/bramble.h"

static int parse_byte (const char *s)
{
    unsigned long val;
    char *endptr;

    errno = 0;
    val = strtoul (s, &endptr, 0);
    if (errno != 0 || *endptr != '\0')
        return -1;
    if (val > 255)
        return -1;

    return val;
}

int led_main (int argc, char *argv[])
{
    int fd;

    if (argc != 2 && argc != 6)
        die ("Usage: bramble led [c | col0 col1 col2 col3 col4]\n");

    if ((fd = nvram_open (O_RDWR)) < 0)
        die ("could not open nvram: %s\n", strerror (errno));

    if (argc == 2) {
        int val;
        uint8_t c;

        if (strlen (argv[1]) == 1 && isprint (argv[1][0]))
            c = argv[1][0];
        else if ((val = parse_byte (argv[1])) >= 0 && isprint (val))
            c = val;
        else
            die ("error parsing character argument\n");
        if (nvram_write (fd, NVRAM_MATRIX_CHAR_ADDR, &c, 1) < 0)
            die ("nvram write: %s\n", strerror (errno));
    }
    else {
        int val;
        uint8_t cols[NVRAM_MATRIX_COLUMNS_SIZE];
        int i;

        /* 5x7 matrix: argument is 5 bytes of 7 bits each
         */
        for (i = 0; i < NVRAM_MATRIX_COLUMNS_SIZE; i++) {
            if ((val = parse_byte (argv[i + 1])) < 0 || val >= 128)
                die ("error parsing column byte %d\n", i);
            cols[i] = val;
        }
        if (nvram_write (fd,
                         NVRAM_MATRIX_COLUMNS_ADDR,
                         cols,
                         NVRAM_MATRIX_COLUMNS_SIZE) < 0)
            die ("nvram write: %s\n", strerror (errno));
    }

    close (fd);

    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

