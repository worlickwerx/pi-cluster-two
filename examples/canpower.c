#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ev.h>

#include "canmgr_proto.h"
#include "canmgr_dump.h"
#include "address.h"
#include "lxcan.h"

static ev_io can_watcher;
static ev_timer timeout_watcher;
static struct ev_loop *loop;

static uint8_t addr_mod;
static uint8_t addr_node;

static int s;

static void can_cb (EV_P_ ev_io *w, int revents)
{
    struct canmgr_frame fr;

    if (lxcan_recv (s, &fr) < 0) {
        fprintf (stderr, "lxcan_recv: %m\n");
        return;
    }
    if (fr.dst != addr_node)
        return; // not addressed to me
    if (fr.object != CANOBJ_TARGET_POWER)
        return;
    if (fr.type != CANMGR_TYPE_ACK && fr.type != CANMGR_TYPE_NAK)
        return;
    if (fr.type == CANMGR_TYPE_NAK) {
        printf ("%.2x,%.2x: NAK to power request\n",
                fr.module, fr.node);
        ev_break (EV_A_ EVBREAK_ONE);
    } else {
        printf ("%.2x,%.2x: power request successful\n",
                fr.module, fr.node);
        ev_break (EV_A_ EVBREAK_ONE);
    }
}

static void timeout_cb (EV_P_ ev_timer *w, int revents)
{
    printf ("Timeout waiting for response\n");
    ev_break (EV_A_ EVBREAK_ONE);
}

static void send_power_request (uint8_t mod, uint8_t node, uint8_t val)
{
    struct canmgr_frame fr;

    fr.pri = 1;
    fr.dst = node | 0x10;
    fr.src = addr_node;

    fr.xpri = 1;
    fr.type = CANMGR_TYPE_WO;
    fr.node = fr.dst;
    fr.module = mod;
    fr.object = CANOBJ_TARGET_POWER;
    fr.data[0] = val;
    fr.dlen = 1;

    if (lxcan_send (s, &fr) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
}

int main (int argc, char *argv[])
{
    int m, n;

    if (argc != 3) {
        fprintf (stderr, "Usage: canpower m,n 0|1|2\n");
        exit (1);
    }
    if (sscanf (argv[1], "%x,%x", &m, &n) != 2
            ||  m < 0 || m >= 0x10 || n < 0 || n >= 0x10) {
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

    send_power_request (m, n, strtoul (argv[2], NULL, 10));

    loop = EV_DEFAULT;

    ev_io_init (&can_watcher, can_cb, s, EV_READ);
    ev_io_start (loop, &can_watcher);

    ev_timer_init (&timeout_watcher, timeout_cb, 3., 0.);
    ev_timer_start (loop, &timeout_watcher);

    ev_run (loop, 0);

    lxcan_close (s);

    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
