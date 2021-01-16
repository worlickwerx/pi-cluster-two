/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _FIRMWARE_CANMSG_H
#define _FIRMWARE_CANMSG_H

struct canmsg_raw {
    uint32_t        msgid;          // Message ID
    uint8_t         length;         // Data length
    uint8_t         data[8];        // Received data
    bool            xmsgidf;        // Extended message flag
    bool            rtrf;           // RTR flag
};

#endif /* !_FIRMWARE_CANMSG_H */
