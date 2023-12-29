/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

/* console.c - helper for conman
 *
 * This program is designed to be spawned by conman to manage one console
 * connection to a slot, e.g. in conman.conf:
 *
 *   console name="picl0" dev="/usr/local/bin/bramble conman-helper 0"
 *   console name="picl1" dev="/usr/local/bin/bramble conman-helper 1"
 *   console name="picl2" dev="/usr/local/bin/bramble conman-helper 2"
 *   ...
 *
 * Any little problem such as a lost ACK is treated as a fatal error
 * by this program.  Conman restarts the helper automatically, and is
 * smart enough not to keep trying if there are enough consecutive failures.
 * It should be safe to configure conman for a full crate even if some
 * slots are unpopulated.
 *
 * N.B. although currently all slots stream data to the same object,
 * multiple instances of conman-helper can co-exist on the same node without
 * confusing streams because we filter messages on the sending address.
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdlib.h>
#include <unistd.h>
#include <endian.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <ev.h>

#include "src/libbramble/bramble.h"

#define CAN_TIMEOUT 0.5         // ack/nack timeout on WO
#define CAN_RECV_TIMEOUT 5.0    // eot timeout on DAT messages

static int              srcaddr;
static int              dstaddr;

static int              can_fd;

static int              console_objid = CANMSG_OBJ_CONSOLESEND;

static ev_io            can_watcher;
static ev_io            stdin_watcher;
static ev_timer         timeout_watcher;
static ev_timer         recv_timeout_watcher;

const char              *timeout_message;

static int              stdin_flags;

static struct ev_loop   *loop;

void restore_stdin (void)
{
    (void)fcntl (STDIN_FILENO, F_SETFL, stdin_flags);
}

/* This timer is started each time we send a request that expects an ACK/NAK
 * to be received by can_cb().  If 'timeout_message' is non-NULL, it is
 * printed to stderr.  This can work since we are never waiting for more
 * than one ACK.
 */
static void timeout_cb (EV_P_ ev_timer *w, int revents)
{
    if (!timeout_message)
        timeout_message = "CAN timeout";
    die ("%s\n", timeout_message);
}

/* Send the connect message, enabling the remote end to start sending data.
 * If the remote slot already has a connection, it is silently overwritten
 * by the new one.
 * N.B. it is possible to recieve a DAT before the console ACK if the
 * can hardware reorders messages due to auto retransmission, and that DAT
 * needs to be ACKed.
 */
static void console_connect (void)
{
    uint8_t data[] = { console_objid };
    struct canmsg msg = { 0 };

    msg.pri = 1;
    msg.src = srcaddr;
    msg.dst = dstaddr;
    msg.type = CANMSG_TYPE_WO;
    msg.object = CANMSG_OBJ_CONSOLECONN;
    msg.dlen = sizeof (data);
    memcpy (msg.data, data, msg.dlen);
    if (can_send (can_fd, &msg) < 0)
        die ("can send error: %s\n", strerror (errno));

    timeout_message = "CAN timeout waiting for connect ACK";
    ev_timer_set (&timeout_watcher, CAN_TIMEOUT, 0.);
    ev_timer_start (loop, &timeout_watcher);
}

/* Self contained write object that causes the remote end to stop sending.
 */
static void console_disconnect (void)
{
    struct canobj *obj;
    uint8_t data[] = { console_objid };

    if (!(obj = canobj_openfd (can_fd, srcaddr, dstaddr,
                               CANMSG_OBJ_CONSOLEDISC))
            || canobj_write (obj, data, sizeof (data)) < 0)
        die ("write CONSOLEDISC: %s", strerror (errno));
    canobj_close (obj);
}

static void console_send_dat (void *data, int len)
{
    struct canmsg msg = { 0 };

    msg.pri = 1;
    msg.src = srcaddr;
    msg.dst = dstaddr;
    msg.type = CANMSG_TYPE_DAT;
    msg.object = CANMSG_OBJ_CONSOLERECV;
    msg.eot = 1; // all messages going this direction are ACKed
    memcpy (msg.data, data, len);
    msg.dlen = len;
    if (can_send (can_fd, &msg) < 0)
        die ("can send error: %s\n", strerror (errno));

    timeout_message = "CAN timeout waiting for data ACK";
    ev_timer_set (&timeout_watcher, CAN_TIMEOUT, 0.);
    ev_timer_start (loop, &timeout_watcher);
}

static void console_send_ack (void)
{
    struct canmsg msg = { 0 };

    msg.pri = 1;
    msg.src = srcaddr;
    msg.dst = dstaddr;
    msg.type = CANMSG_TYPE_ACK;
    msg.object = console_objid;
    if (can_send (can_fd, &msg) < 0)
        die ("can send error: %s\n", strerror (errno));
}

static struct canmsg chunk[16];
static size_t chunklen = sizeof (chunk) / sizeof (chunk[0]);

static bool chunk_is_complete (bool holes_ok)
{
    int i;

    for (i = 0; i < chunklen; i++) {
        if (!holes_ok && chunk[i].dlen == 0)
            break;
        if (chunk[i].eot)
            return true;
    }
    return false;
}

static void write_chunk_to_stdout (void)
{
    int i;

    for (i = 0; i < chunklen; i++) {
        if (chunk[i].dlen > 0) {
            if (write (STDOUT_FILENO, chunk[i].data, chunk[i].dlen) < 0)
                die ("error writing to stdout: %s\n", strerror (errno));
            if (chunk[i].eot)
                break;
        }
    }
}

static void handle_recv_dat (struct canmsg *msg)
{
    if (!ev_is_active (&recv_timeout_watcher)) {
        ev_timer_set (&recv_timeout_watcher, CAN_RECV_TIMEOUT, 0.);
        ev_timer_start (loop, &recv_timeout_watcher);
    }
    chunk[msg->seq] = *msg;

    if (chunk_is_complete (false)) {
        write_chunk_to_stdout ();
        memset (chunk, 0, sizeof (chunk));
        console_send_ack ();
        ev_timer_stop (loop, &recv_timeout_watcher);
    }
}

static void recv_timeout_cb (EV_P_ ev_timer *w, int revents)
{
    if (chunk_is_complete (true)) {
        write_chunk_to_stdout ();
        memset (chunk, 0, sizeof (chunk));
        console_send_ack ();
    }
    else { // if eot not received in timeout, let conman restart connection
        console_disconnect ();
        die ("receive timeout\n");
    }
}

/* Restart stdin watcher when previous DAT message has been ACKed.
 * Assume problems with the connection if a NAK is received, and that
 * conman will respawn a helper which will reconnect.
 */
static void handle_send_ack (struct canmsg *msg)
{
    if (msg->type == CANMSG_TYPE_NAK)
        die ("received NAK to sent console data\n");

    ev_timer_stop (loop, &timeout_watcher);
    ev_io_start (loop, &stdin_watcher);
}

static void handle_connect_ack (struct canmsg *msg)
{
    if (msg->type == CANMSG_TYPE_NAK)
        die ("received NAK - connection is probably broken\n");

    ev_timer_stop (loop, &timeout_watcher);
    ev_io_start (loop, &stdin_watcher);
}

static void can_cb (EV_P_ ev_io *w, int revents)
{
    struct canmsg msg;
    uint32_t netseq;
    uint32_t seq;
    double t;

    if (can_recv (can_fd, &msg) < 0)
        die ("CAN receive error: %s\n", strerror (errno));
    if (msg.dst == srcaddr && msg.src == dstaddr) {
        if (msg.object == console_objid
                && msg.type == CANMSG_TYPE_DAT) {
            handle_recv_dat (&msg);
        }
        else if (msg.object == CANMSG_OBJ_CONSOLERECV
                && (msg.type == CANMSG_TYPE_ACK
                 || msg.type == CANMSG_TYPE_NAK)) {
            handle_send_ack (&msg);
        }
        else if (msg.object == CANMSG_OBJ_CONSOLECONN
                && (msg.type == CANMSG_TYPE_ACK
                 || msg.type == CANMSG_TYPE_NAK)) {
            handle_connect_ack (&msg);
        }
        /* Ignore other object ID's - a power response might pop up here
         * during conman &R processing, for example.
         */
    }
}

/* Read up to 8 bytes from stdin and send it to the CAN console.
 * Stop stdin watcher pending receipt of ACK.
 */
static void stdin_cb (EV_P_ ev_io *w, int revents)
{
    uint8_t data[8];
    int len;

    if ((len = read (STDIN_FILENO, data, sizeof (data))) < 0) {
        if (errno == EAGAIN)
            return;
        die ("read error on stdin: %s\n", strerror (errno));
    }
    if (len == 0) {
        console_disconnect ();
        die ("EOF on stdin\n");
    }

    console_send_dat (data, len);
    ev_io_stop (loop, &stdin_watcher);
}

int console_main (int argc, char *argv[])
{
    int slot;
    char *endptr;

    if (argc != 2 && argc != 3)
        die ("Usage: bramble conman-helper SLOT [SRCADDR]\n");

    errno = 0;
    slot = strtol (argv[1], &endptr, 10);
    if (errno != 0 || *endptr != '\0' || slot < 0 || slot > 16)
        die ("error parsing target slot number");
    dstaddr = slot | CANMSG_ADDR_CONTROL;

    if (argc == 3) {
        errno = 0;
        srcaddr = strtol (argv[2], &endptr, 16);
        if (errno != 0 || *endptr != '\0' || srcaddr < 0 || srcaddr > 0x3f)
            die ("error parsing hexadecimal SRCADDR from command line");
    }
    else {
        if ((slot = slot_get ()) < 0) {
            warn ("WARNING: failed to read slot: %s, assuming CAN address 02\n",
                  strerror (errno));
            srcaddr = 0x02;
        }
        else
            srcaddr = slot | CANMSG_ADDR_COMPUTE;
    }

    /* Receive only broadcast messages, and messages directed to this node
     * from the target slot's control processor.
     */
    struct can_filter rfilter[] = {
        {   .can_id = (dstaddr << CANMSG_SRC_SHIFT)
                    | (srcaddr << CANMSG_DST_SHIFT),
            .can_mask = CANMSG_SRC_MASK | CANMSG_DST_MASK,
        },
        {   .can_id = CANMSG_ADDR_BROADCAST << CANMSG_DST_SHIFT,
            .can_mask = CANMSG_DST_MASK,
        },
    };

    if ((can_fd = can_open_with (BRAMBLE_CAN_INTERFACE, rfilter, 2)) < 0)
        die ("%s: %s\n", BRAMBLE_CAN_INTERFACE, strerror (errno));

    /* Set stdin to non-blocking.
     * An atexit() handler restores previous modes.
     */
    if ((stdin_flags = fcntl (STDIN_FILENO, F_GETFL)) < 0)
        die ("fcntl F_GETFL on stdin: %s\n", strerror (errno));
    if (atexit (restore_stdin) != 0)
        die ("atexit: %s", strerror (errno));
    if (fcntl (STDIN_FILENO, F_SETFL, stdin_flags | O_NONBLOCK) < 0)
        die ("fcntl F_SETFL on stdin: %s\n", strerror (errno));

    loop = EV_DEFAULT;

    ev_io_init (&can_watcher, can_cb, can_fd, EV_READ);
    ev_io_start (loop, &can_watcher);

    ev_io_init (&stdin_watcher, stdin_cb, 0, EV_READ);

    ev_timer_init (&timeout_watcher, timeout_cb, 0., 0.);
    ev_timer_init (&recv_timeout_watcher, recv_timeout_cb, 0., 0.);

    console_connect (); // takes ACK in event loop

    ev_run (loop, 0);

    console_disconnect (); // self-contained

    close (can_fd);

    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

