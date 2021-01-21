/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _BRAMBLE_CANMSG_H
#define _BRAMBLE_CANMSG_H

#include <stdint.h>
#include <stdbool.h>

struct canmsg_raw {
    uint32_t        msgid;          // Message ID
    uint8_t         length;         // Data length
    uint8_t         data[8];        // Received data
    bool            xmsgidf;        // Extended message flag
    bool            rtrf;           // RTR flag
};

#endif /* !_BRAMBLE_CANMSG_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
