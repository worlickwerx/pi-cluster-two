/* SPDX-License-Identifier: GPL-3.0-or-later */

struct canmsg_raw {
    uint32_t        msgid;          // Message ID
    uint32_t        fmi;            // Filter index
    uint8_t         length;         // Data length
    uint8_t         data[8];        // Received data
    uint8_t         xmsgidf:1;      // Extended message flag
    uint8_t         rtrf:1;         // RTR flag
    uint8_t         fifo:1;         // RX Fifo 0 or 1
};

