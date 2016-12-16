#ifndef _CANMGR_PROTO_H
#define _CANMGR_PROTO_H

// See Meiko CS/2 "Overview of the Control Area Network (CAN)"
// The LCAN/XCAN/GCAN and extended addressing/routing scheme described
// in that document was borrowed here.

// LCAN addressing within a module:
// compute board: 00 01 02 03 04 05 06 07 08 09 0a 0b
// compute mgr:   10 11 12 13 14 15 16 17 18 19 1a 1b
// module mgr:    1d

// CAN id (11 byte):
//   pri:1 dst:5 src:5  (priority: 0=high, 1=low)
struct canmgr_id {
    uint16_t pri:1;
    uint16_t dst:5;
    uint16_t src:5;
};
#define CANMGR_DST_MASK (0b01111100000)

// CAN packet (4 byte header + 0-4 byte data)
//   pri:1 type:3 cluster:6 module:6 node:6 object:10 data:32
struct canmgr_pkt {
    uint32_t pri:1;
    uint32_t type:3;
    uint32_t cluster:6;         // gcan address
    uint32_t module:6;          // mcan address
    uint32_t node:6;            // lcan address
    uint32_t object:10;
    uint8_t data[4];
};

// special 6-bit canmgr addresses for cluster/module/node
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
    CANOBJ_LED_IDENTIFY = 0,    // 1 byte (0=LED Off, 1=LED blinking)
    CANOBJ_TARGET_POWER = 1,    // 1 byte (0=5V off, 1=5V on)
    CANOBJ_TARGET_SHUTDOWN = 2, // 1 byte (0=normal, 1=begin shutdown sequence)
    CANOBJ_TARGET_RESET = 3,    // 1 byte (0=run, 1=hold in reset, 2=toggle)

    CANOBJ_TARGET_CONSOLECONN = 4, // 4 bytes: c:6 m:6 n:6 t:2 obj:10
    CANOBJ_TARGET_CONSOLEDISC = 5, // 4 bytes: c:6 m:6 n:6 t:2 obj:10

    // todo:

    // heartbeat

    // jtag/openocd protocol for remote gdb, etc..
};


/* Encode/decode canmgr_pkt to/from 8 byte CAN payload.
 * decode returns the number of bytes in the data field (0-4), or -1 on error.
 * encode returns the number of bytes to send (4-8), or -1 on error.
 */
int canmgr_pkt_decode (struct canmgr_pkt *pkt, uint8_t *buf, int buf_len);
int canmgr_pkt_encode (struct canmgr_pkt *pkt, int data_len,
                       uint8_t *buf, int buf_len);

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

#endif /* _CANMGR_PROTO_H */

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

