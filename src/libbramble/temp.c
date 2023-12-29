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
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "src/libbramble/bramble.h"

#define PATH_TEMP "/sys/class/thermal/thermal_zone0/temp"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

int temp_read (double *temp)
{
    FILE *f;
    char buf[32];
    double val;

    if (!(f = fopen (PATH_TEMP, "r")))
        return -1;
    if (fgets (buf, sizeof (buf), f) == NULL) {
        fclose (f);
        return -1;
    }
    fclose (f);

    errno = 0;
    val = strtod (buf, NULL);
    if (errno > 0)
        return -1;
    if (temp)
        *temp = val / 1000;
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

