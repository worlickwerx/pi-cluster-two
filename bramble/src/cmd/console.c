/* SPDX-License-Identifier: GPL-3.0-or-later */

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

static int              my_slot;
static int              target_slot;

static int              can_fd;

static int              console_objid;
static bool             connected;

static ev_io            can_watcher;
static ev_io            stdin_watcher;

static int              stdin_flags;

static struct ev_loop   *loop;


void restore_stdin (void)
{
    (void)fcntl (STDIN_FILENO, F_SETFL, stdin_flags);
}

/* If the remote slot already has a connection, it is silently overwritten
 * by the new one, thus the helper can die without disconnecting, and
 * successfully reconnect when respawned.
 */
static void console_connect (void)
{
    uint8_t data[] = { 0, my_slot | 0x10, console_objid }; // mod, node, objid
    struct canmsg_v1 msg = { 0 };
    struct canmsg_raw raw;

    msg.pri = 1;
    msg.src = my_slot | 0x10;
    msg.dst = target_slot;
    msg.xpri = 1;
    msg.type = CANMSG_V1_TYPE_WO;
    msg.object = CANMSG_V1_OBJ_CONSOLECONN;
    msg.dlen = sizeof (data);
    memcpy (msg.data, data, msg.dlen);
    if (canmsg_v1_encode (&msg, &raw) < 0)
        die ("error encoding message\n");
    if (can_send (can_fd, &raw) < 0)
        die ("can send error: %s\n", strerror (errno));
}

static void console_disconnect (void)
{
    struct canobj *obj;
    uint8_t data[] = { 0, my_slot | 0x10, console_objid }; // mod, node, objid

    if (!(obj = canobj_open_with (can_fd, my_slot,
                                  target_slot, CANMSG_V1_OBJ_CONSOLEDISC))
            || canobj_write (obj, data, sizeof (data)) < 0)
        die ("write CONSOLEDISC: %s", strerror (errno));
    canobj_close (obj);
}

static void console_send_dat (void *data, int len)
{
    struct canmsg_v1 msg = { 0 };
    struct canmsg_raw raw;

    msg.pri = 1;
    msg.src = my_slot | 0x10;
    msg.dst = target_slot;
    msg.xpri = 1;
    msg.type = CANMSG_V1_TYPE_DAT;
    msg.object = CANMSG_V1_OBJ_CONSOLERECV;
    memcpy (msg.data, data, len);
    msg.dlen = len;
    if (canmsg_v1_encode (&msg, &raw) < 0)
        die ("error encoding message\n");
    if (can_send (can_fd, &raw) < 0)
        die ("can send error: %s\n", strerror (errno));
}

static void console_send_ack (void)
{
    struct canmsg_v1 msg = { 0 };
    struct canmsg_raw raw;

    msg.pri = 1;
    msg.src = my_slot | 0x10;
    msg.dst = target_slot;
    msg.xpri = 1;
    msg.type = CANMSG_V1_TYPE_ACK;
    msg.object = console_objid;
    if (canmsg_v1_encode (&msg, &raw) < 0)
        die ("error encoding message\n");
    if (can_send (can_fd, &raw) < 0)
        die ("can send error: %s\n", strerror (errno));
}

static void handle_recv_dat (struct canmsg_v1 *msg)
{
    int n;

    if ((n = write (STDOUT_FILENO, msg->data, msg->dlen)) < 0)
        die ("error writing to stdout: %s\n", strerror (errno));
    console_send_ack ();
}

/* Restart stdin watcher when previous DAT message has been ACKed.
 * Assume problems iwth the connection if a NAK is received, and that
 * conman will respawn a helper which will reconnect.
 */
static void handle_send_ack (struct canmsg_v1 *msg)
{
    if (msg->type == CANMSG_V1_TYPE_NAK)
        die ("received NAK - connection is probably broken\n");
    ev_io_start (loop, &stdin_watcher);
}

static void handle_connect_ack (struct canmsg_v1 *msg)
{
    if (!connected) {
        ev_io_start (loop, &stdin_watcher);
        connected = true;
    }
}

static void can_cb (EV_P_ ev_io *w, int revents)
{
    struct canmsg_v1 msg;
    struct canmsg_raw raw;
    uint32_t netseq;
    uint32_t seq;
    double t;

    if (can_recv (can_fd, &raw) < 0)
        die ("CAN receive error: %s\n", strerror (errno));
    if (canmsg_v1_decode (&raw, &msg) < 0) {
        fprintf (stderr, "error decoding CAN message\n");
        return; // non-fatal
    }
    if (msg.dst == (my_slot | 0x10) && msg.src == target_slot) {
        if (msg.object == console_objid
                && msg.type == CANMSG_V1_TYPE_DAT) {
            handle_recv_dat (&msg);
        }
        else if (msg.object == CANMSG_V1_OBJ_CONSOLERECV
                && (msg.type == CANMSG_V1_TYPE_ACK
                 || msg.type == CANMSG_V1_TYPE_NAK)) {
            handle_send_ack (&msg);
        }
        else if (msg.object == CANMSG_V1_OBJ_CONSOLECONN
                && (msg.type == CANMSG_V1_TYPE_ACK
                 || msg.type == CANMSG_V1_TYPE_NAK)) {
            handle_connect_ack (&msg);
        }
        else {
            fprintf (stderr, "WARN: ignoring message from taget\n");
        }
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
    if (argc != 2)
        die ("Usage: bramble canping SLOT\n");
    if ((target_slot = slot_parse (argv[1])) < 0)
        die ("error parsing target slot number");

    if ((my_slot = slot_get ()) < 0)
        die ("could not read slot number from i2c\n");

    if ((can_fd = can_open (BRAMBLE_CAN_INTERFACE)) < 0)
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

    /* Set the object ID that the remote slot will use to send data to us.
     * The Main use case is now spawning multiple conman-helper processes
     * on management node, each connected to a different slot.
     * As long as we filter incoming DAT messages on src address, we can
     * use the same object ID for all.  The v1 protocol allows us to pick
     * an ID and send it with the connect message, but all the "dynamic IDs"
     * use a 7 byte payload and its more efficient (and simpler for firmware)
     * to use all 8 payload bytes, so go this way.
     */
    console_objid = CANMSG_V1_OBJ_CONSOLESEND;

    console_connect ();

    loop = EV_DEFAULT;

    ev_io_init (&can_watcher, can_cb, can_fd, EV_READ);
    ev_io_start (loop, &can_watcher);

    ev_io_init (&stdin_watcher, stdin_cb, 0, EV_READ);

    ev_run (loop, 0);

    console_disconnect ();

    close (can_fd);

    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

