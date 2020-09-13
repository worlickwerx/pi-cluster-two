/* SPDX-License-Identifier: GPL-3.0-or-later */

struct canmsg_v1 {
    /* 29 bit id */
    uint16_t pri:1; // 0=high, 1=low
    uint16_t dst:5;
    uint16_t src:5;
    uint32_t xpri:1; // 0=high, 1=low
    uint32_t type:3;
    uint32_t module:6;
    uint32_t node:6;
    uint32_t object:8; // only 2 bits in id; if >= uses data[0]
    /* 8 byte payload */
    uint8_t dlen;
    uint8_t data[8];
};

#define CANMSG_V1_DST_SHIFT (23)
#define CANMSG_V1_DST_MASK (0x1f << CANMSG_V1_DST_SHIFT)

#define CANMSG_V1_LEADER_ADDR (0x1d)
#define CANMSG_V1_BCAST_ADDR (0x1e)
#define CANMSG_V1_NOROUTE_ADDR (0x1f)

enum {
    CANMSG_V1_TYPE_RO = 0,
    CANMSG_V1_TYPE_WO = 1,
    CANMSG_V1_TYPE_WNA = 2,
    CANMSG_V1_TYPE_DAT = 3,
    CANMSG_V1_TYPE_ACK = 4,
    CANMSG_V1_TYPE_NAK = 6,
    CANMSG_V1_TYPE_SIG = 7,
};

enum {
    /* these ids are represented in 2 bit object id,
     * leaving the full data[0:7] for payload
     */
    CANMSG_V1_OBJ_HEARTBEAT     = 0,
    CANMSG_V1_OBJ_CONSOLERECV   = 1,
    CANMSG_V1_OBJ_CONSOLESEND   = 2,

    /* these ids are represented in 2 bits object id + 6 bits data[0],
     * leaving data[1:7] for payload
     */
    CANMSG_V1_OBJ_LED_IDENTIFY  = 3,
    CANMSG_V1_OBJ_POWER         = 4,
    CANMSG_V1_OBJ_ECHO          = 5,
    CANMSG_V1_OBJ_RESET         = 6,
    CANMSG_V1_OBJ_CONSOLECONN   = 7,
    CANMSG_V1_OBJ_CONSOLEDISC   = 8,
    CANMSG_V1_OBJ_CONSOLERING   = 9,
    CANMSG_V1_OBJ_POWER_MEASURE = 10,
    /* 0x80 - 0xff reserved for dynamically allocated CONSOLESEND objects */
    CANMSG_V1_OBJ_CONSOLEBASE = 0x80,
};

int canmsg_v1_decode (const struct canmsg_raw *raw,
                      struct canmsg_v1 *msg);
int canmsg_v1_encode (const struct canmsg_v1 *msg,
                      struct canmsg_raw *raw);
void canmsg_v1_trace (const struct canmsg_v1 *msg);

/*
 * vi:ts=4 sw=4 expandtab
 */
