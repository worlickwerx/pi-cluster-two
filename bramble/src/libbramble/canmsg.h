/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _BRAMBLE_CANMSG_H
#define _BRAMBLE_CANMSG_H

#include <stdint.h>

/* 29-bit CAN ID:
 * +---------------------------------------------------+
 * | pri:1 | dst:6 | src:6 | type:3 | object:8 | seq:5 |
 * +---------------------------------------------------+
 *
 * 6-bit addresses:
 * +-------------------+
 * | device:2 | slot:4 |
 * +-------------------+
 */

struct canmsg {
    uint32_t pri:1; // 0=high, 1=low
    uint32_t dst:6;
    uint32_t src:6;
    uint32_t type:3;
    uint32_t object:8;
    uint32_t seq:5;
    uint8_t dlen;
    uint8_t data[8];
};

#define CANMSG_DST_SHIFT     22
#define CANMSG_DST_MASK      (0x3f << (CANMSG_DST_SHIFT))

#define CANMSG_SRC_SHIFT     16
#define CANMSG_SRC_MASK      (0x3f << (CANMSG_SRC_SHIFT))

enum {
    CANMSG_ADDR_INVALID      = 0x00,
    CANMSG_ADDR_SUPERVISOR   = 0x01,
    CANMSG_ADDR_MANAGEMENT   = 0x02,
    CANMSG_ADDR_BROADCAST    = 0x0f,
    CANMSG_ADDR_CONTROL      = 0x10,
    CANMSG_ADDR_COMPUTE      = 0x20,
    CANMSG_ADDR_COMMS        = 0x30,
};

enum {
    CANMSG_TYPE_RO = 0,
    CANMSG_TYPE_WO = 1,
    CANMSG_TYPE_WNA = 2,
    CANMSG_TYPE_DAT = 3,
    CANMSG_TYPE_ACK = 4,
    CANMSG_TYPE_NAK = 6,
    CANMSG_TYPE_SIG = 7,
};

enum {
    CANMSG_OBJ_ECHO = 0,
    CANMSG_OBJ_POWER = 1,
    CANMSG_OBJ_POWER_MEASURE = 2,
    CANMSG_OBJ_CONSOLECONN = 3,
    CANMSG_OBJ_CONSOLERECV = 4,
    CANMSG_OBJ_CONSOLESEND = 5,
    CANMSG_OBJ_CONSOLEDISC = 6,
};

#endif /* !_BRAMBLE_CANMSG_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
