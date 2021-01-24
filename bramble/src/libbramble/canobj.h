/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _BRAMBLE_CANOBJ_H
#define _BRAMBLE_CANOBJ_H

struct canobj *canobj_open_with (int canfd, int sslot, int dslot, int object);
struct canobj *canobj_open (int slot, int object);
void canobj_close (struct canobj *obj);

int canobj_write (struct canobj *obj, void *data, int len);
int canobj_read (struct canobj *obj, void *data, int len);

#endif /* !_BRAMBLE_CANOBJ_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
