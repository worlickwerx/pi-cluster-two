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
static struct canmgr_hdr console_hdr = {
    .cluster = CANMGR_ADDR_NOROUTE,
    .module = CANMGR_ADDR_NOROUTE,
    .node = CANMGR_ADDR_NOROUTE,
};

void canmgr_setup (uint8_t can_addr)
{
    int i;
    can0_begin (CANMGR_DST_MASK);
    // receive dst=can_addr
    for (i = 0; i < 8; i++)
        can0_setfilter (can_addr<<5, i);
    myaddr = can_addr;
}

int can_recv (struct canmgr_frame *fr)
{
    struct rawcan_frame raw;

    if (!can0_read (&raw.id, &raw.dlen, &raw.data[0]))
        return -1;
    if (canmgr_decode (fr, &raw) < 0)
        return -1;
    return 0;
}

int can_send (struct canmgr_frame *fr)
{
    struct rawcan_frame raw;

    if (canmgr_encode (fr, &raw) < 0)
        return -1;
    if (!can0_write (raw.id, raw.dlen, &raw.data[0], 100))
        return -1;
    return 0;
}

void canmgr_ack (struct canmgr_frame *fr, int type, uint8_t *data, int len)
{
    struct canmgr_frame ack;

    ack.id.src = fr->id.dst;
    ack.id.dst = fr->id.src;
    ack.id.pri = fr->id.pri;

    ack.hdr = fr->hdr;
    ack.hdr.type = type;
    memcpy (&ack.data[0], data, len);
    ack.dlen = len;

    can_send (&ack);
}

void canobj_led_identify (struct canmgr_frame *fr)
{
    uint8_t val;

    switch (fr->hdr.type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 1)
                goto nak;
            identify_set (fr->data[0]);
            canmgr_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
            if (fr->dlen != 0)
                goto nak;
            identify_get (&val);
            canmgr_ack (fr, CANMGR_TYPE_ACK, &val, 1);
            break;
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_power (struct canmgr_frame *fr)
{
    uint8_t val;

    switch (fr->hdr.type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 1)
                goto nak;
            target_power_set (fr->data[0]);
            canmgr_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
            if (fr->dlen != 0)
                goto nak;
            target_power_get (&val);
            canmgr_ack (fr, CANMGR_TYPE_ACK, &val, 1);
            break;
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_reset (struct canmgr_frame *fr)
{
    uint8_t val;

    switch (fr->hdr.type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 1)
                goto nak;
            if (fr->data[0] == 1 || fr->data[0] == 0)
                target_reset_set (fr->data[0]);
            else
                target_reset_pulse ();
            canmgr_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
            if (fr->dlen != 0)
                goto nak;
            target_reset_get (&val);
            canmgr_ack (fr, CANMGR_TYPE_ACK, &val, 1); 
            break;
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_consoleconn (struct canmgr_frame *fr)
{
    uint8_t val[4];

    switch (fr->hdr.type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 4)
                goto nak;
            if (!CONSOLE_UNCONNECTED (&fr->hdr))
                goto nak;
            /* data contains new console object - copy it there */
            if (canmgr_decode_hdr (&console_hdr, fr->data, fr->dlen) < 0)
                goto nak;
            canmgr_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
            if (fr->dlen != 0)
                goto nak;
            if (canmgr_encode_hdr (&console_hdr, val, 4) < 0)
                goto nak;
            canmgr_ack (fr, CANMGR_TYPE_ACK, val, 4); 
            break;
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_consoledisc (struct canmgr_frame *fr)
{
    struct canmgr_hdr old;

    switch (fr->hdr.type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 4)
                goto nak;
            if (canmgr_decode_hdr (&old, fr->data, fr->dlen) < 0)
                goto nak;
            if (canmgr_compare_hdr (&old, &console_hdr) != 0)
                goto nak;
            console_hdr.cluster = CANMGR_ADDR_NOROUTE;
            console_hdr.module = CANMGR_ADDR_NOROUTE;
            console_hdr.node = CANMGR_ADDR_NOROUTE;
            canmgr_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_unknown (struct canmgr_frame *fr)
{
    switch (fr->hdr.type) {
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
        case CANMGR_TYPE_DAT:
            canmgr_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
            break;
        default:
            break;
    }
}

        
void canmgr_dispatch (struct canmgr_frame *fr)
{
    switch (fr->hdr.object) {
        case CANOBJ_LED_IDENTIFY:
            canobj_led_identify (fr);
            break;
        case CANOBJ_TARGET_POWER:
            canobj_target_power (fr);
            break;
        case CANOBJ_TARGET_RESET:
            canobj_target_power (fr);
            break;
        case CANOBJ_TARGET_CONSOLECONN:
            canobj_target_consoleconn (fr);
            break;
        case CANOBJ_TARGET_CONSOLEDISC:
            canobj_target_consoledisc (fr);
            break;
        default:
            canobj_unknown (fr);
            break;
    }
}

void canmgr_update (void)
{
    struct canmgr_frame fr;

    if (can0_available ()) {
        activity_pulse ();
        if (can_recv (&fr) == 0)
            canmgr_dispatch (&fr);
    }
    if (target_console_available ()) {
        fr.dlen = target_console_recv (&fr.data[0], 4);
        if (!CONSOLE_UNCONNECTED(&console_hdr) && fr.dlen > 0) {
            fr.id.src = 0;
            fr.id.dst = 0;
            fr.id.pri = 1;
            fr.hdr = console_hdr;
            can_send (&fr);
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

