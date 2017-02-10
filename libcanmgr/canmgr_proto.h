#ifndef _CANMGR_PROTO_H
#define _CANMGR_PROTO_H

// Inspired by Meiko CS/2 "Overview of the Control Area Network (CAN)"

// CAN addressing within a module:
// compute board: 00 01 02 03 04 05 06 07 08 09 0a 0b
// compute mgr:   10 11 12 13 14 15 16 17 18 19 1a 1b
// module mgr:    1d

#define CANMGR_DST_SHIFT  (23)
#define CANMGR_DST_MASK (0x1f<<CANMGR_DST_SHIFT)

#define CANMGR_MODULE_CTRL  (0x1d)

// header uses first byte of CAN payload (optionally)
struct canmgr_frame {
    /* 29 byte id */
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

struct rawcan_frame {
    uint32_t id;
    uint8_t dlen;
    uint8_t data[8];
};

// special 6-bit canmgr addresses for module/node
enum {
    CANMGR_ADDR_NOROUTE = 0b111111,
    CANMGR_ADDR_BCAST   = 0b111110,
};

// Message types:
enum {
    CANMGR_TYPE_RO = 0,         // read object (master only)
    CANMGR_TYPE_WO = 1,         // write object (master only)
    CANMGR_TYPE_WNA = 2,        // write object without ack (master only)
    CANMGR_TYPE_DAT = 3,        // data
    CANMGR_TYPE_ACK = 4,        // write acknowledge
    CANMGR_TYPE_NAK = 6,        // negative write acknowledge
    CANMGR_TYPE_SIG = 7,        // send a signal (slaves), obj in data:10
};

// canmgr objects
enum {
   // N.B. objects 0, 1, 2 are encoded in the id, thus allow 8 bytes of payload
    CANOBJ_HEARTBEAT = 0,
    CANOBJ_TARGET_CONSOLERECV = 1, // recv 0-8 bytes of data from cancon
    CANOBJ_TARGET_CONSOLESEND = 2, // send 0-8 bytes of data to cancon

    CANOBJ_LED_IDENTIFY = 3,    // 1 byte (0=off, 1=on, 2=blinking)
    CANOBJ_TARGET_POWER = 4,    // 1 byte (0=off, 1=on, 2=shutdown, 3=toggle)
    CANOBJ_ECHO = 5,            // payloaded echoed back (ping)
    CANOBJ_TARGET_RESET = 6,    // 1 byte (0=run, 1=hold in reset, 2=toggle)

    CANOBJ_TARGET_CONSOLECONN = 7, // 3 bytes: m, n, obj_offset
    CANOBJ_TARGET_CONSOLEDISC = 8, // 3 bytes: m, n, obj_offset
    CANOBJ_TARGET_CONSOLERING = 9, // 3 bytes: m, n, obj_offset

    CANOBJ_TARGET_CONSOLEBASE = 0x80,
    // 0x80 - 0xff reserved fro more CONSOLESEND objects
    // these should be unique on the sending node
};

#define CONSOLE_UNCONNECTED(h) \
    ((h)->module == 0x3f && (h)->node == 0x3f)
int canmgr_decode (struct canmgr_frame *fr, struct rawcan_frame *raw);
int canmgr_encode (struct canmgr_frame *fr, struct rawcan_frame *raw);

int canmgr_maxdata (int object);

#endif /* _CANMGR_PROTO_H */

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

