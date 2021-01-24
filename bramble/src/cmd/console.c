/* SPDX-License-Identifier: GPL-3.0-or-later */

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
 * Note: this program registers a hardwired object ID for received data.
 * This is safe despite there being multiple helpers running on the same
 * node because we filter (in software) messages not sent by the target slot.
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

#define CAN_TIMEOUT 0.5

static int              my_slot;
static int              target_slot;

static int              can_fd;

/* v1 protocol allows different object IDs to be registered, but only
 * this one can use the full 8 bytes of payload.  See note above.
 */
static int              console_objid = CANMSG_V1_OBJ_CONSOLESEND;

static ev_io            can_watcher;
static ev_io            stdin_watcher;
static ev_timer         timeout_watcher;

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

    timeout_message = "CAN timeout waiting for connect ACK";
    ev_timer_set (&timeout_watcher, CAN_TIMEOUT, 0.);
    ev_timer_start (loop, &timeout_watcher);
}

/* Self contained write object that causes the remote end to stop sending.
 */
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

    timeout_message = "CAN timeout waiting for data ACK";
    ev_timer_set (&timeout_watcher, CAN_TIMEOUT, 0.);
    ev_timer_start (loop, &timeout_watcher);
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
        die ("received NAK to sent console data\n");

    ev_timer_stop (loop, &timeout_watcher);
    ev_io_start (loop, &stdin_watcher);
}

static void handle_connect_ack (struct canmsg_v1 *msg)
{
    if (msg->type == CANMSG_V1_TYPE_NAK)
        die ("received NAK - connection is probably broken\n");

    ev_timer_stop (loop, &timeout_watcher);
    ev_io_start (loop, &stdin_watcher);
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

    loop = EV_DEFAULT;

    ev_io_init (&can_watcher, can_cb, can_fd, EV_READ);
    ev_io_start (loop, &can_watcher);

    ev_io_init (&stdin_watcher, stdin_cb, 0, EV_READ);

    ev_timer_init (&timeout_watcher, timeout_cb, 0., 0.);

    console_connect (); // takes ACK in event loop

    ev_run (loop, 0);

    console_disconnect (); // self-contained

    close (can_fd);

    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

