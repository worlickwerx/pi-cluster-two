/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include "canmsg.h"

/* Initialize the can bus at 1mbps.
 */
void canbus_init (void);

/* Add a filter for received messages based on 29 bit id and mask.
 * 'nr' may be 0 through 27 (there are 28 filter banks).
 */
void canbus_filter_set (uint32_t nr, uint32_t id, uint32_t mask);

/* Send a raw CAN message.
 * Return 0 on success or -1 on failure (no xmit mailboxes available).
 */
int canbus_send (struct canmsg_raw *raw);

/* Receive a raw can message, waiting up to 'timeout' milliseconds for
 * one to arrive.  timeout=0 returns immediately, timeout=-1 waits forever.
 * Return 0 on success or -1 on failure (no message available after timeout).
 */
int canbus_recv (struct canmsg_raw *raw, int timeout);
