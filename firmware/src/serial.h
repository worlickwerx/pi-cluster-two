/* SPDX-License-Identifier: GPL-3.0-or-later */

void serial_init (void);

int serial_recv (unsigned char *buf, int len, int timeout);

void serial_send (const unsigned char *buf, int len);

/*
 * vi:ts=4 sw=4 expandtab
 */
