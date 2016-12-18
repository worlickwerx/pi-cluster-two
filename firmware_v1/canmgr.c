#include <WProgram.h>
#include "can.h"
#include "canmgr_proto.h"
#include "canmgr.h"
#include "activity.h"
#include "identify.h"
#include "target_power.h"
#include "target_reset.h"
#include "target_console.h"

static struct canmgr_pkt canmgr_console_id = {
    .pri = 1,
    .type = CANMGR_TYPE_DAT,
    .cluster = 0,
    .module = 0,
    .node = 8,
    .object = 0x0382,
};
static uint8_t canmgr_console_connected = 1;

void canmgr_setup (uint32_t can_addr)
{
    int i;
    can0_begin (CANMGR_DST_MASK);
    // receive dst=can_addr
    for (i = 0; i < 8; i++)
        can0_setfilter (can_addr<<5, i);
    canmgr_console_connected = 0;
}

void canmgr_ack (struct canmgr_id *id, struct canmgr_pkt *pkt, int type)
{
    struct canmgr_id ackid;
    struct canmgr_pkt ackpkt = *pkt;
    uint32_t rawid; 
    uint8_t buf[8];
    int len;

    ackid.src = id->dst;
    ackid.dst = id->src;
    ackid.pri = id->pri;
    ackpkt.type = type;

    rawid = ackid.src;
    rawid |= ackid.dst<<5;
    rawid |= ackid.pri<<10;

    len = canmgr_pkt_encode (&ackpkt, 0, buf, 8);
    can0_write (rawid, len, buf, 100);
}

void canmgr_dispatch (struct canmgr_id *id, struct canmgr_pkt *pkt, uint8_t len)
{
    switch (pkt->object) {
        case CANOBJ_LED_IDENTIFY:
            if (len == 1)
                identify_set (pkt->data[0]);
            // FIXME: send ACK
            break;
        case CANOBJ_TARGET_POWER:
            if (len == 1 && (pkt->data[0] == 0 || pkt->data[0] == 1)) {
                target_power_set (pkt->data[0]);
                canmgr_ack (id, pkt, CANMGR_TYPE_ACK);
            } else
                canmgr_ack (id, pkt, CANMGR_TYPE_NAK);
            break;
        case CANOBJ_TARGET_RESET:
            if (len == 1) {
                if (pkt->data[0] == 1 || pkt->data[0] == 0)
                    target_reset_set (pkt->data[0]);
                else
                    target_reset_pulse ();
            }
            // FIXME: send ACK
            break;
        case CANOBJ_TARGET_CONSOLECONN:
            break;
        case CANOBJ_TARGET_CONSOLEDISC:
            break;
    }
}

void canmgr_update (void)
{
    uint8_t buf[8];
    uint32_t rawid;
    uint8_t len, cons_len;
    struct canmgr_pkt pkt;
    struct canmgr_id id;

    if (can0_available ()) {
        activity_pulse ();
        if (can0_read (&rawid, &len, buf)) {
            id.src = rawid & 0x1f;
            id.dst = (rawid>>5) & 0x1f;
            id.pri = (rawid>>10)&1;
            int data_len = canmgr_pkt_decode (&pkt, buf, len);
            if (data_len >= 0)
                canmgr_dispatch (&id, &pkt, len - 4);
        }
    }
    if (target_console_available ()) {
        activity_pulse ();
        cons_len = target_console_recv (&canmgr_console_id.data[0], 4);
        int len = canmgr_pkt_encode (&canmgr_console_id, cons_len, buf, 8);
        if (len >= 0) {
            // FIXME set id
            can0_write (rawid, len, buf, 100);
        }
    }
}

void canmgr_finalize (void)
{
    can0_end ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

