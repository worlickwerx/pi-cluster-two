/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#ifndef _FIRMWARE_BLINK_H
#define _FIRMWARE_BLINK_H

void vApplicationStackOverflowHook (xTaskHandle *task, signed portCHAR *name);

void blink_init (void);

#endif /* !_FIRMWARE_BLINK_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
