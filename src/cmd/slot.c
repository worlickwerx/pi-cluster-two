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
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include "src/libbramble/bramble.h"

int slot_main (int argc, char *argv[])
{
    int slot;

    if (argc != 1)
        die ("Usage: bramble slot");

    if ((slot = slot_get ()) < 0)
        die ("could not read slot number\n");
    printf ("%d\n", slot);

    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

