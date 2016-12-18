#include <sys/socket.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/can.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "canmgr_proto.h"
#include "lxcan.h"

static const uint8_t myaddr = 0; // lie

int main (int argc, char *argv[])
{
    struct canmgr_frame in, out;
    struct rawcan_frame raw;
    int c, m, n;
    int s;

    if (argc != 3) {
        fprintf (stderr, "Usage: canpower c,m,n 0|1\n");
        exit (1);
    }
    if (sscanf (argv[1], "%d,%d,%d", &c, &m, &n) != 3
            || c < 0 || c >= 0x10 || m < 0 || m >= 0x10 || c < 0 || c >= 0x10) {
        fprintf (stderr, "improperly specified target\n");
        exit (1);
    }
    /* construct request
     * for now, no routing - cluster and module are ignored
     */
    in.id.pri = 1;
    in.id.dst = n | 0x10;
    in.id.src = myaddr;

    in.hdr .pri = 1;
    in.hdr.type = CANMGR_TYPE_WO;
    in.hdr.node = in.id.dst;
    in.hdr.module = m;
    in.hdr.cluster = c;
    in.hdr.object = CANOBJ_TARGET_POWER;
    in.data[0] = strtoul (argv[2], NULL, 10); /* 0=off, 1=on */
    in.dlen = 1;

    /* encode raw frame
     */
    if (canmgr_encode (&in, &raw) < 0) {
        fprintf (stderr, "error encoding CAN frame\n");
        exit (1);
    }

    /* send frame on can0
     * wait for ack/nak response
     */
    if ((s = lxcan_open ("can0")) < 0) {
        fprintf (stderr, "lxcan_open: %m\n");
        exit (1);
    }
    if (lxcan_send (s, &raw) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
    // TODO timeout
    for (;;) {
        if (lxcan_recv (s, &raw) < 0) {
            fprintf (stderr, "lxcan_recv: %m\n");
            exit (1);
        }
        if (canmgr_decode (&out, &raw) < 0) {
            fprintf (stderr, "canmgr_decode error, ignoring packet\n");
            continue;
        }
        if (out.id.src != in.id.dst || out.id.dst != in.id.src)
            continue;
        if (out.hdr.object != CANOBJ_TARGET_POWER)
            continue;
        if (out.hdr.cluster != in.hdr.cluster
                || out.hdr.module != in.hdr.module
                || out.hdr.node != in.hdr.node)
            continue;
        if (out.hdr.type == CANMGR_TYPE_ACK) {
            fprintf (stderr, "OK\n");
            exit (0);
        }
        if (out.hdr.type == CANMGR_TYPE_NAK) {
            fprintf (stderr, "Received NAK response\n");
            exit (1);
        }
    }
    can_close (s);

    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
