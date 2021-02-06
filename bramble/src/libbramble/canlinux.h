/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _BRAMBLE_CANLINUX_H
#define _BRAMBLE_CANLINUX_H

#include <linux/can/raw.h>

#include "canmsg.h"

#define BRAMBLE_CAN_INTERFACE   "can0"

/* Open the specified CAN interface and install filters to match
 * msg->dst == myaddr OR msg->dst == CANMSG_ADDR_BROADCAST.
 */
int can_open (const char *name, int myaddr);

/* If rfilter is NULL or len == 0, all messages are received.
 * Otherwise, messages are filtered according to the 'rfilter' array
 * with 'len' elements.
 */
int can_open_with (const char *name, const struct can_filter *rfilter, int len);

int can_recv (int fd, struct canmsg *msg);
int can_send (int fd, struct canmsg *msg);
int can_recv_timeout (int fd, struct canmsg *msg, double timeout);

#endif /* !_BRAMBLE_CANLINUX_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
