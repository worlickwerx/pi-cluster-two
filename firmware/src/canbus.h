/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#ifndef _FIRMWARE_CANBUS_H
#define _FIRMWARE_CANBUS_H

#include <stdbool.h>
#include "canmsg.h"

/* Initialize the can bus at 1mbps.
 */
void canbus_init (void);

/* Add a filter for received messages based on 29 bit id and mask.
 * 'nr' may be 0 through 27 (there are 28 filter banks).
 */
void canbus_filter_set (uint32_t nr, uint32_t id, uint32_t mask);

/* Send a CAN message.
 * Return 0 on success or -1 on failure (no xmit mailboxes available).
 */
int canbus_send (const struct canmsg *msg);

/* Receive a CAN message, waiting up to 'timeout' milliseconds for
 * one to arrive.  timeout=0 returns immediately, timeout=-1 waits forever.
 * Return 0 on success or -1 on failure (no message available after timeout).
 */
int canbus_recv (struct canmsg *msg, int timeout);

#endif /* !_FIRMWARE_CANBUS_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
