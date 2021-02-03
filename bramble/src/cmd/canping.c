/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <unistd.h>
#include <stdlib.h>
#include <endian.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <ev.h>

#include "src/libbramble/bramble.h"

static int srcaddr;
static int dstaddr;

static int can_fd;

static ev_io    can_watcher;
static ev_timer timer_watcher;
static struct ev_loop *loop;

static uint32_t sequence;
#define TIMESTAMP_SLOTS 256
static double timestamp[TIMESTAMP_SLOTS];

/* Print a line on receipt of each ACK.
 */
static void can_cb (EV_P_ ev_io *w, int revents)
{
    struct canmsg msg;
    uint32_t netseq;
    uint32_t seq;
    double t;

    if (can_recv (can_fd, &msg) < 0)
        die ("can_recv: %s\n", strerror (errno));
    if (msg.dst != srcaddr || msg.src != dstaddr)
        return;
    if (msg.object != CANMSG_OBJ_ECHO)
        return;
    if (msg.type != CANMSG_TYPE_NAK && msg.type != CANMSG_TYPE_ACK)
        return;
    if (msg.type == CANMSG_TYPE_NAK) {
        printf ("%d bytes from %.2x: NAK\n", msg.dlen, msg.src);
        return;
    }
    if (msg.dlen < 4) {
        printf ("%d bytes from %.2x: ACK (runt)\n", msg.dlen, msg.src);
        return;
    }
    memcpy (&netseq, &msg.data[0], 4);
    seq = be32toh (netseq);
    if (seq > sequence) {
        printf ("%d bytes from %.2x: seq=%d (higher than sent)\n",
                msg.dlen, msg.src, (int)seq);
        return;
    }
    t = monotime_since (timestamp[seq % TIMESTAMP_SLOTS]);
    printf ("%d bytes from %.2x: seq=%d time=%.1f ms\n",
            msg.dlen, msg.src, (int)seq, t * 1E3);
}

/* Timer periodically sends a sequenced echo request,
 * regardless of whether the last one has been ACKed or not.
 * Current timestamp is written to timestamp[sequence % TIMESTAMP_SLOTS]
 * for timing the ACK, which will contain the sequence number.
 */
static void timer_cb (EV_P_ ev_timer *w, int revents)
{
    struct canmsg msg = { 0 };
    uint32_t netseq = htobe32 (sequence);

    msg.pri = 1;
    msg.src = srcaddr;
    msg.dst = dstaddr;
    msg.type = CANMSG_TYPE_WO;
    msg.object = CANMSG_OBJ_ECHO;
    msg.dlen = 7;
    memcpy (&msg.data[0], &netseq, 4);
    memset (&msg.data[4], 0xff, 3);

    timestamp[sequence++ % TIMESTAMP_SLOTS] = monotime ();

    if (can_send (can_fd, &msg) < 0)
        die ("error sending CAN message\n");
}

int canping_main (int argc, char *argv[])
{
    int slot;
    char *endptr;

    if (argc != 2 && argc != 3)
        die ("Usage: bramble canping ADDR [SRCADDR]\n");

    errno = 0;
    dstaddr = strtol (argv[1], &endptr, 16);
    if (errno != 0 || *endptr != '\0' || dstaddr < 0 || dstaddr > 0x3f)
        die ("error parsing hexadecimal ADDR from command line\n");

    if (argc == 3) {
        errno = 0;
        srcaddr = strtol (argv[2], &endptr, 16);
        if (errno != 0 || *endptr != '\0' || srcaddr < 0 || srcaddr > 0x3f)
            die ("error parsing hexadecimal SRCADDR from command line\n");
    }
    else {
        if ((slot = slot_get ()) < 0) {
            warn ("WARNING: i2c: %s, assuming CAN address 02\n",
                  strerror (errno));
            srcaddr = 0x02;
        }
        else
            srcaddr = slot | CANMSG_ADDR_COMPUTE;
    }

    if ((can_fd = can_open (BRAMBLE_CAN_INTERFACE)) < 0)
        die ("%s: %s\n", BRAMBLE_CAN_INTERFACE, strerror (errno));

    loop = EV_DEFAULT;

    ev_io_init (&can_watcher, can_cb, can_fd, EV_READ);
    ev_io_start (loop, &can_watcher);

    ev_timer_init (&timer_watcher, timer_cb, 0., 1.);
    ev_timer_start (loop, &timer_watcher);

    ev_run (loop, 0);

    close (can_fd);

    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

