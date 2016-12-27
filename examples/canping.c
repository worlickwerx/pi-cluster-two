#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ev.h>

#include "canmgr_proto.h"
#include "lxcan.h"
#include "address.h"
#include "monotime.h"

uint8_t addr_mod, addr_node;
uint8_t targ_mod, targ_node;
uint32_t seq = 0;
int s;
#define nslots (16)
static double timestamp[nslots];

static ev_io can_watcher;
static ev_timer timeout_watcher;
static struct ev_loop *loop;

static void can_cb (EV_P_ ev_io *w, int revents)
{
    struct canmgr_frame fr;
    uint32_t nseq;

    if (lxcan_recv (s, &fr) < 0) {
        fprintf (stderr, "lxcan_recv: %m\n");
        return;
    }
    if (fr.dst != addr_node)
        return; // not addressed to me
    if (fr.object != CANOBJ_ECHO)
        return;
    if (fr.type != CANMGR_TYPE_ACK && fr.type != CANMGR_TYPE_NAK)
        return;

    if (fr.type == CANMGR_TYPE_NAK) {
        printf ("%d bytes from %.2x,%.2x: NAK\n", fr.dlen, fr.module, fr.node);
    } else if (fr.dlen < 4) {
        printf ("%d bytes from %.2x,%.2x: ACK (runt)\n",
                fr.dlen, fr.module, fr.node);
    } else {
        memcpy (&nseq, &fr.data[0], 4);
        printf ("%d bytes from %.2x,%.2x: seq=%d time=%.1f ms\n",
                fr.dlen,
                fr.module, fr.node,
                ntohl (nseq),
                monotime_since (timestamp[ntohl (nseq) % nslots]) * 1000);
    }
}

static void timeout_cb (EV_P_ ev_timer *w, int revents)
{
    struct canmgr_frame in;
    uint32_t nseq = htonl (seq);

    in.pri = 1;
    in.dst = targ_node;
    in.src = addr_node;

    in.xpri = 1;
    in.type = CANMGR_TYPE_WO;
    in.node = in.dst;
    in.module = targ_mod;
    in.object = CANOBJ_ECHO;
    memcpy (&in.data[0], &nseq, 4);
    memset (&in.data[4], 0xff, 3);
    in.dlen = 7;

    timestamp[seq++ % nslots]= monotime ();
    if (lxcan_send (s, &in) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
}


int main (int argc, char *argv[])
{
    char dump[80];

    if (argc != 2) {
        fprintf (stderr, "Usage: canping m,n\n");
        exit (1);
    }
    if (sscanf (argv[1], "%x,%x", &targ_mod, &targ_node) != 2) {
        fprintf (stderr, "improperly specified target\n");
        exit (1);
    }
    if (can_address_get (&addr_mod, &addr_node) < 0) {
        fprintf (stderr, "could not read GPIO lines: %m\n");
        exit (1);
    }
    if ((s = lxcan_open ("can0")) < 0) {
        fprintf (stderr, "lxcan_open: %m\n");
        exit (1);
    }
    loop = EV_DEFAULT;

    ev_io_init (&can_watcher, can_cb, s, EV_READ);
    ev_io_start (loop, &can_watcher);

    ev_timer_init (&timeout_watcher, timeout_cb, 0., 1.);
    ev_timer_start (loop, &timeout_watcher);

    ev_run (loop, 0);

    lxcan_close (s);

    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
