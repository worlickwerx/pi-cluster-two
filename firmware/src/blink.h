/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _FIRMWARE_BLINK_H
#define _FIRMWARE_BLINK_H

void vApplicationStackOverflowHook (xTaskHandle *task, signed portCHAR *name);

void blink_init (void);

#endif /* !_FIRMWARE_BLINK_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
