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

/* Console state
 */
static struct canmgr_frame console_connected = {
    .module = CANMGR_ADDR_NOROUTE,
    .node = CANMGR_ADDR_NOROUTE,
};
static int console_lastsent_needack = 0;
static int console_ringdump = 0;

void canmgr_setup (uint8_t can_addr)
{
    int i;
    can0_begin (CANMGR_DST_MASK);
    // receive dst=can_addr
    for (i = 0; i < 8; i++)
        can0_setfilter (can_addr<<CANMGR_DST_SHIFT, i);
    myaddr = can_addr;
}

int can_recv (struct canmgr_frame *fr, uint16_t timeout_ms)
{
    struct rawcan_frame raw;

    if (!can0_read (&raw.id, &raw.dlen, &raw.data[0], timeout_ms))
        return -1;
    if (canmgr_decode (fr, &raw) < 0)
        return -1;
    if (fr->dst != myaddr)
        return -1;
    return 0;
}

int can_send (struct canmgr_frame *fr, uint16_t timeout_ms)
{
    struct rawcan_frame raw;

    if (canmgr_encode (fr, &raw) < 0)
        return -1;
    if (!can0_write (raw.id, raw.dlen, &raw.data[0], timeout_ms))
        return -1;
    return 0;
}

void canmgr_ack (struct canmgr_frame *fr, int type, uint8_t *data, int len)
{
    int tmpaddr = fr->src;
    fr->src = fr->dst;
    fr->dst = tmpaddr;
    fr->pri = 0; // high priority
    fr->type = type;

    if (len > canmgr_maxdata (fr->object))
        return;
    memcpy (fr->data, data, len);
    fr->dlen = len;

    can_send (fr, 100);
}

int canmgr_console_send_ready (void)
{
    if (CONSOLE_UNCONNECTED (&console_connected))
        return 0;
    if (console_lastsent_needack)
        return 0;
    return 1;
}

void canmgr_console_send (uint8_t *buf, int len)
{
    if (!canmgr_console_send_ready ())
        return;
    if (len > canmgr_maxdata (console_connected.object))
        return;
    console_connected.dlen = len;
    memcpy (console_connected.data, buf, len);
    if (can_send (&console_connected, 100) < 0)
        return;
    console_lastsent_needack = 1;
}

void canobj_led_identify (struct canmgr_frame *fr)
{
    uint8_t val;

    switch (fr->type) {
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

    switch (fr->type) {
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

    switch (fr->type) {
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
    uint8_t val[3];

    switch (fr->type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 3)
                goto nak;
            /* Prepare reusable outgoing DAT packet for console output
             */
            console_connected.pri = 1;
            console_connected.dst = fr->data[1];
            console_connected.src = myaddr;
            console_connected.xpri = 1;
            console_connected.type = CANMGR_TYPE_DAT;
            console_connected.module = fr->data[0];
            console_connected.node = fr->data[1];
            console_connected.object = fr->data[2];
            target_console_reset (); // dump only new data
            canmgr_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
            if (fr->dlen != 0)
                goto nak;
            val[0] = console_connected.module;
            val[1] = console_connected.node;
            val[2] = console_connected.object;
            canmgr_ack (fr, CANMGR_TYPE_ACK, val, 3);
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
    switch (fr->type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 3 || console_connected.module != fr->data[0]
                              || console_connected.node != fr->data[1]
                              || console_connected.object != fr->data[2])
                goto nak;
            console_connected.node = CANMGR_ADDR_NOROUTE;
            console_connected.module = CANMGR_ADDR_NOROUTE;
            canmgr_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            console_lastsent_needack = 0;
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

void canobj_target_consolering (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 3 || console_connected.module != fr->data[0]
                              || console_connected.node != fr->data[1]
                              || console_connected.object != fr->data[2])
                goto nak;
            target_console_history_reset ();
            console_ringdump = 1;
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

void canobj_target_consolerecv (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_DAT:
            target_console_send (fr->data, fr->dlen);
            canmgr_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
            goto nak;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_consolesend (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
        case CANMGR_TYPE_DAT:
            goto nak;
        case CANMGR_TYPE_ACK:
        case CANMGR_TYPE_NAK:
            console_lastsent_needack = 0;
            break;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_unknown (struct canmgr_frame *fr)
{
    switch (fr->type) {
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
    switch (fr->object) {
        case CANOBJ_LED_IDENTIFY:
            canobj_led_identify (fr);
            break;
        case CANOBJ_TARGET_POWER:
            canobj_target_power (fr);
            break;
        case CANOBJ_TARGET_RESET:
            canobj_target_reset (fr);
            break;
        case CANOBJ_TARGET_CONSOLECONN:
            canobj_target_consoleconn (fr);
            break;
        case CANOBJ_TARGET_CONSOLEDISC:
            canobj_target_consoledisc (fr);
            break;
        case CANOBJ_TARGET_CONSOLERECV:
            canobj_target_consolerecv (fr);
            break;
        case CANOBJ_TARGET_CONSOLERING:
            canobj_target_consolering (fr);
            break;
        default:
            if (fr->object == console_connected.object) {
                canobj_target_consolesend (fr);
                break;
            }
            canobj_unknown (fr);
            break;
    }
}

void canmgr_update (void)
{
    struct canmgr_frame fr;

    if (can0_available ()) {
        activity_pulse ();
        if (can_recv (&fr, 0) == 0)
            canmgr_dispatch (&fr);
    }
    if (canmgr_console_send_ready ()) {
        uint8_t buf[8];
        int max = canmgr_maxdata (console_connected.object);
        int len = 0;

        if (max > sizeof (buf))
            max = sizeof (buf);
        if (console_ringdump) {
            if ((len = target_console_history_next (buf, max)) == 0)
                console_ringdump = 0; // EOF
        } else {
            len = target_console_recv (buf, max);
        }
        if (len > 0) {
            activity_pulse ();
            canmgr_console_send (buf, len);
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
