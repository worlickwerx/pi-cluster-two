#include <WProgram.h>
#include "can.h"
#include "canmgr_proto.h"
#include "canmgr.h"
#include "activity.h"
#include "identify.h"
#include "target_power.h"
#include "target_reset.h"
#include "target_console.h"

void canmgr_setup (uint32_t can_addr)
{
    int i;
    can0_begin (CANMGR_DST_MASK);
    // receive dst=can_addr
    for (i = 0; i < 8; i++)
        can0_setfilter (can_addr<<5, i);
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

void canmgr_dispatch (struct canmgr_frame *fr)
{
    switch (fr->hdr.object) {
        case CANOBJ_LED_IDENTIFY:
            if (fr->dlen == 1) {
                identify_set (fr->data[0]);
                canmgr_ack (fr, CANMGR_TYPE_ACK);
            }
                canmgr_ack (fr, CANMGR_TYPE_NAK);
            break;
        case CANOBJ_TARGET_POWER:
            if (fr->dlen == 1 && (fr->data[0] == 0 || fr->data[0] == 1)) {
                target_power_set (fr->data[0]);
                canmgr_ack (fr, CANMGR_TYPE_ACK);
            } else
                canmgr_ack (fr, CANMGR_TYPE_NAK);
            break;
        case CANOBJ_TARGET_RESET:
            if (fr->dlen== 1) {
                if (fr->data[0] == 1 || fr->data[0] == 0)
                    target_reset_set (fr->data[0]);
                else
                    target_reset_pulse ();
                canmgr_ack (fr, CANMGR_TYPE_ACK);
            } else
                canmgr_ack (fr, CANMGR_TYPE_NAK);
            break;
        case CANOBJ_TARGET_CONSOLECONN:
            break;
        case CANOBJ_TARGET_CONSOLEDISC:
            break;
    }
}

void canmgr_update (void)
{
    struct rawcan_frame raw;
    struct canmgr_frame in;

    if (can0_available ()) {
        activity_pulse ();
        if (can_recv (&raw) < 0 || canmgr_decode (&in, &raw) < 0)
            return;
        canmgr_dispatch (&in);
    }
}

void canmgr_finalize (void)
{
    can0_end ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

