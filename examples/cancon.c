/* cancon - simple CAN console client */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "canmgr_proto.h"
#include "canmgr_dump.h"
#include "lxcan.h"

static const uint8_t myaddr = 0; // lie
static struct canmgr_hdr cons;
static int s;
static int c, m, n;

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

    if (lxcan_send (s, &ack) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
}

void console_connect (void)
{
    struct canmgr_frame in;
    
    in.id.pri = 1;
    in.id.dst = n | 0x10;
    in.id.src = myaddr;

    in.hdr.pri = 1;
    in.hdr.type = CANMGR_TYPE_WO;
    in.hdr.node = in.id.dst;
    in.hdr.module = m;
    in.hdr.cluster = c;
    in.hdr.object = CANOBJ_TARGET_CONSOLECONN;
    if (canmgr_encode_hdr (&cons, &in.data[0], 4) < 0) {
        fprintf (stderr, "error encoding console object\n");
        exit (1);
    }
    in.dlen = 4;
    if (lxcan_send (s, &in) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
    /* FIXME collect ack */
}

void console_disconnect (void)
{
    struct canmgr_frame in;
    
    in.id.pri = 1;
    in.id.dst = n | 0x10;
    in.id.src = myaddr;

    in.hdr.pri = 1;
    in.hdr.type = CANMGR_TYPE_WO;
    in.hdr.node = in.id.dst;
    in.hdr.module = m;
    in.hdr.cluster = c;
    in.hdr.object = CANOBJ_TARGET_CONSOLEDISC;
    if (canmgr_encode_hdr (&cons, &in.data[0], 4) < 0) {
        fprintf (stderr, "error encoding console object\n");
        exit (1);
    }
    in.dlen = 4;
    if (lxcan_send (s, &in) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
    /* FIXME: collect ack */
}

void canobj_console_recv (struct canmgr_frame *fr)
{
    switch (fr->hdr.type) {
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
            goto nak;
        case CANMGR_TYPE_DAT:
            printf ("%.*s", fr->dlen, fr->data);
            fflush (stdout); // FIXME
            canmgr_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void sigint_handler (int arg)
{
    console_disconnect ();
    exit (0);
}

int main (int argc, char *argv[])
{
    struct canmgr_frame in, out;

    if (argc != 2) {
        fprintf (stderr, "Usage: cancon c,m,n\n");
        exit (1);
    }
    if (sscanf (argv[1], "%d,%d,%d", &c, &m, &n) != 3
            || c < 0 || c >= 0x10 || m < 0 || m >= 0x10 || c < 0 || c >= 0x10) {
        fprintf (stderr, "improperly specified target\n");
        exit (1);
    }
    /* construct console object (payload)
     */
    cons.pri = 1;
    cons.type = CANMGR_TYPE_DAT;
    cons.node = myaddr;
    cons.module = 0;
    cons.cluster = 0;
    cons.object = 0x10f; // FIXME: make it unique

    if ((s = lxcan_open ("can0")) < 0) {
        fprintf (stderr, "lxcan_open: %m\n");
        exit (1);
    }
    console_connect ();

    signal (SIGINT, sigint_handler);

    for (;;) {
        if (lxcan_recv (s, &out) < 0) {
            fprintf (stderr, "lxcan_recv: %m\n");
            exit (1);
        }
        if (out.id.dst != myaddr)
            continue;
        switch (out.hdr.object) {
            default:
                if (out.hdr.object == cons.object) {
                    canobj_console_recv (&out);
                    break;
                }
                break;
        }
    }
    console_disconnect ();
    lxcan_close (s);

    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
