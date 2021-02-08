/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _FIRMWARE_SERIAL_H
#define _FIRMWARE_SERIAL_H

void serial_init (void);

/* N.B. serial_send() and serial_recv() may only be used from
 * one task (each) because FreeRTOS stream buffers are used internally.
 * Timeouts are in milliseconds (0=return immediately, -1=wait forever).
 * Task yields while waiting.
 */
int serial_recv (unsigned char *buf, int len, int timeout);
int serial_send (const unsigned char *buf, int len, int timeout);

int serial_recv_available (void);

void serial_rx_enable (void);
void serial_rx_disable (void);

#endif /* !_FIRMWARE_SERIAL_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
