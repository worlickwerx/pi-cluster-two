/* cancon - simple CAN console client */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "canmgr_proto.h"
#include "canmgr_dump.h"
#include "lxcan.h"

static const uint8_t mynode = 0; // lie
static const uint8_t mymod = 0; // lie
static int object = CANOBJ_TARGET_CONSOLESEND;
static int s;
static int m, n;

void canmgr_ack (struct canmgr_frame *fr, int type)
{
    int tmpaddr = fr->src;
    fr->src = fr->dst;
    fr->dst = tmpaddr;
    fr->type = type;
    fr->pri = 0; // acks are high priority
    fr->dlen = 0;

    if (lxcan_send (s, fr) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
}

void console_request (int obj)
{
    struct canmgr_frame in;
    
    in.pri = 1;
    in.dst = n | 0x10;
    in.src = mynode;

    in.xpri = 1;
    in.type = CANMGR_TYPE_WO;
    in.node = in.dst;
    in.module = m;
    in.object = obj;
    in.data[0] = mymod;
    in.data[1] = mynode;
    in.data[2] = object;
    in.dlen = 3;
    if (lxcan_send (s, &in) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
    /* FIXME collect ack */
}

void canobj_console_recv (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
            goto nak;
        case CANMGR_TYPE_DAT:
            printf ("%.*s", fr->dlen, fr->data);
            fflush (stdout); // FIXME
            canmgr_ack (fr, CANMGR_TYPE_ACK);
            break;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK);
}

void sigint_handler (int arg)
{
    console_request (CANOBJ_TARGET_CONSOLEDISC);
    exit (0);
}

int main (int argc, char *argv[])
{
    struct canmgr_frame in, out;

    if (argc != 2) {
        fprintf (stderr, "Usage: cancon m,n\n");
        exit (1);
    }
    if (sscanf (argv[1], "%d,%d", &m, &n) != 2
            || m < 0 || m >= 0x10 || n < 0 || n >= 0x10) {
        fprintf (stderr, "improperly specified target\n");
        exit (1);
    }
    /* construct console object (payload)
     */

    // FIXME: object should be unique
    // Use CANOBJ_TARGET_CONSOLESEND for the common case,
    // but if that's in use, select from range CANOBJ_TARGET_CONSOLEBASE - 0xff

    if ((s = lxcan_open ("can0")) < 0) {
        fprintf (stderr, "lxcan_open: %m\n");
        exit (1);
    }
    console_request (CANOBJ_TARGET_CONSOLECONN);

    signal (SIGINT, sigint_handler);

    for (;;) {
        if (lxcan_recv (s, &out) < 0) {
            fprintf (stderr, "lxcan_recv: %m\n");
            exit (1);
        }
        if (out.dst != mynode)
            continue;
        switch (out.object) {
            default:
                if (out.object == object) {
                    canobj_console_recv (&out);
                    break;
                }
                break;
        }
    }
    console_request (CANOBJ_TARGET_CONSOLEDISC);
    lxcan_close (s);

    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
