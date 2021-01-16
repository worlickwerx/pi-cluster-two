/* SPDX-License-Identifier: GPL-3.0-or-later */

void serial_init (void);

/* N.B. serial_send() and serial_recv() may only be used from
 * one task (each) because FreeRTOS stream buffers are used internally.
 * Timeouts are in milliseconds (0=return immediately, -1=wait forever).
 * Task yields while waiting.
 */
int serial_recv (unsigned char *buf, int len, int timeout);
int serial_send (const unsigned char *buf, int len, int timeout);

void serial_rx_enable (void);
void serial_rx_disable (void);

/*
 * vi:ts=4 sw=4 expandtab
 */
