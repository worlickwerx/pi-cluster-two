#include <WProgram.h>
#include "can.h"
#include "canmgr_proto.h"
#include "canmgr.h"
#include "activity.h"
#include "identify.h"
#include "target_power.h"
#include "target_reset.h"
#include "target_console.h"

static uint8_t myaddr;
static uint8_t console_connected = 0;
static struct canmgr_hdr console_hdr;

void canmgr_setup (uint8_t can_addr)
{
    int i;
    can0_begin (CANMGR_DST_MASK);
    // receive dst=can_addr
    for (i = 0; i < 8; i++)
        can0_setfilter (can_addr<<5, i);
    myaddr = can_addr;
}

int can_recv (struct rawcan_frame *raw)
{
    if (!can0_read (&raw->id, &raw->dlen, &raw->data[0]))
        return -1;
    return 0;
}

int can_send (struct rawcan_frame *raw)
{
    if (!can0_write (raw->id, raw->dlen, &raw->data[0], 100))
        return -1;
    return 0;
}

void canmgr_ack (struct canmgr_frame *fr, int type)
{
    struct canmgr_frame ack;
    struct rawcan_frame raw;

    ack.id.src = fr->id.dst;
    ack.id.dst = fr->id.src;
    ack.id.pri = fr->id.pri;

    ack.hdr = fr->hdr;
    ack.hdr.type = type;
    ack.dlen = 0;

    if (canmgr_encode (&ack, &raw) < 0)
        return;
    can_send (&raw);
}

int console_connect (uint8_t *data, int len)
{
    if (console_connected)
        return CANMGR_TYPE_NAK;
    if (canmgr_decode_hdr (&console_hdr, data, len) < 0)
        return CANMGR_TYPE_NAK;
    console_connected = 1;
    return CANMGR_TYPE_ACK;
}

int console_disconnect (uint8_t *data, int len)
{
    struct canmgr_hdr hdr;

    if (canmgr_decode_hdr (&hdr, data, len) < 0)
        return CANMGR_TYPE_NAK;
    if (canmgr_compare_hdr (&hdr, &console_hdr) != 0)
        return CANMGR_TYPE_NAK;
    console_connected = 0;
    return CANMGR_TYPE_ACK;
}

void canmgr_dispatch (struct canmgr_frame *fr)
{
    int acktype = CANMGR_TYPE_NAK;

    switch (fr->hdr.object) {
        case CANOBJ_LED_IDENTIFY:
            if (fr->hdr.type == CANMGR_TYPE_WO) {
                if (fr->dlen == 1) {
                    identify_set (fr->data[0]);
                    acktype = CANMGR_TYPE_ACK;
                }
                canmgr_ack (fr, acktype);
            }
            break;
        case CANOBJ_TARGET_POWER:
            if (fr->hdr.type == CANMGR_TYPE_WO) {
                if (fr->dlen == 1) {
                    target_power_set (fr->data[0]);
                    acktype = CANMGR_TYPE_ACK;
                }
                canmgr_ack (fr, acktype);
            }
            break;
        case CANOBJ_TARGET_RESET:
            if (fr->hdr.type == CANMGR_TYPE_WO) {
                if (fr->dlen == 1) {
                    if (fr->data[0] == 1 || fr->data[0] == 0)
                        target_reset_set (fr->data[0]);
                    else
                        target_reset_pulse ();
                    acktype = CANMGR_TYPE_ACK;
                }
                canmgr_ack (fr, acktype);
            }
            break;
        case CANOBJ_TARGET_CONSOLECONN:
            if (fr->hdr.type == CANMGR_TYPE_WO) {
                if (fr->dlen == 4 && !console_connected)
                    acktype = console_connect (fr->data, fr->dlen);
                canmgr_ack (fr, acktype);
            }
            break;
        case CANOBJ_TARGET_CONSOLEDISC:
            if (fr->hdr.type == CANMGR_TYPE_WO) {
                if (fr->dlen == 4)
                    acktype = console_disconnect (fr->data, fr->dlen);
            }
            canmgr_ack (fr, acktype);
            break;
    }
}

void canmgr_update (void)
{
    struct rawcan_frame raw;
    struct canmgr_frame fr;


    if (can0_available ()) {
        activity_pulse ();
        if (can_recv (&raw) >= 0 && canmgr_decode (&fr, &raw) >= 0)
            canmgr_dispatch (&fr);
    }
    if (console_connected && target_console_available ()) {
        fr.id.src = 0;
        fr.id.dst = 0;
        fr.id.pri = 1;

        fr.hdr.cluster = 0;
        fr.hdr.module = 0;
        fr.hdr.node = myaddr;
        fr.hdr.type = CANMGR_TYPE_DAT;
        fr.hdr.object = 0;
        fr.dlen = target_console_recv (&fr.data[0], 4);

        canmgr_encode (&fr, &raw);
        can_send (&raw);
    }
}

void canmgr_finalize (void)
{
    can0_end ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

