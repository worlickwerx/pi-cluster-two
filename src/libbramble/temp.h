/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#ifndef _BRAMBLE_TEMP_H
#define _BRAMBLE_TEMP_H

/* Read temperature in C
 * Returns 0 on success, -1 on error.
 */
int temp_read (double *temp);

#endif /* !_BRAMBLE_TEMP */

// vi:ts=4 sw=4 expandtab
