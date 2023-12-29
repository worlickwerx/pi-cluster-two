/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#ifndef _BRAMBLE_CANOBJ_H
#define _BRAMBLE_CANOBJ_H

struct canobj *canobj_openfd (int canfd, int srcaddr, int dstaddr, int object);
void canobj_close (struct canobj *obj);

int canobj_write (struct canobj *obj, void *data, int len);
int canobj_read (struct canobj *obj, void *data, int len);

#endif /* !_BRAMBLE_CANOBJ_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
