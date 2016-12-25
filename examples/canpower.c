#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "canmgr_proto.h"
#include "canmgr_dump.h"
#include "lxcan.h"

static const uint8_t myaddr = 0; // lie

int main (int argc, char *argv[])
{
    struct canmgr_frame in, out;
    int m, n;
    int s;
    char dump[80]; 

    if (argc != 3) {
        fprintf (stderr, "Usage: canpower m,n 0|1|2\n");
        exit (1);
    }
    if (sscanf (argv[1], "%d,%d", &m, &n) != 2
            ||  m < 0 || m >= 0x10 || n < 0 || n >= 0x10) {
        fprintf (stderr, "improperly specified target\n");
        exit (1);
    }
    /* construct request
     * for now, no routing - cluster and module are ignored
     */
    in.pri = 1;
    in.dst = n | 0x10;
    in.src = myaddr;

    in.xpri = 1;
    in.type = CANMGR_TYPE_WO;
    in.node = in.dst;
    in.module = m;
    in.object = CANOBJ_TARGET_POWER;
    in.data[0] = strtoul (argv[2], NULL, 10); /* 0=off, 1=on, 2=shutdown */
    in.dlen = 1;

    /* send frame on can0
     * wait for ack/nak response
     */
    if ((s = lxcan_open ("can0")) < 0) {
        fprintf (stderr, "lxcan_open: %m\n");
        exit (1);
    }
    if (lxcan_send (s, &in) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
    // TODO timeout
    for (;;) {
        if (lxcan_recv (s, &out) < 0) {
            fprintf (stderr, "lxcan_recv: %m\n");
            exit (1);
        }
        if (out.src != in.dst || out.dst != in.src)
            continue;
        if (out.object != CANOBJ_TARGET_POWER)
            continue;
        if (out.module != in.module || out.node != in.node)
            continue;
        if (out.type == CANMGR_TYPE_ACK) {
            fprintf (stderr, "OK\n");
            exit (0);
        }
        if (out.type == CANMGR_TYPE_NAK) {
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
