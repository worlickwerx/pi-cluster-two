/* cancon - simple CAN console client */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <ev.h>
#include "canmgr_proto.h"
#include "canmgr_dump.h"
#include "lxcan.h"

static const uint8_t mynode = 0; // lie
static const uint8_t mymod = 0; // lie
static int myobject = CANOBJ_TARGET_CONSOLESEND;
static int s;
static int m, n;

static int expecting_recv_ack = 0;

static struct termios saved_tio;
static struct termios new_tio;

static ev_io can_watcher;
static ev_io stdin_watcher;
static struct ev_loop *loop;

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
    in.data[2] = myobject;
    in.dlen = 3;
    if (lxcan_send (s, &in) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
}

void identify_request (uint8_t state)
{
    struct canmgr_frame in;

    in.pri = 1;
    in.dst = n | 0x10;
    in.src = mynode;

    in.xpri = 1;
    in.type = CANMGR_TYPE_WO;
    in.node = in.dst;
    in.module = m;
    in.object = CANOBJ_LED_IDENTIFY;
    in.data[0] = state;
    in.dlen = 1;
    if (lxcan_send (s, &in) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
}

void reset_request (uint8_t val)
{
    struct canmgr_frame in;

    in.pri = 1;
    in.dst = n | 0x10;
    in.src = mynode;

    in.xpri = 1;
    in.type = CANMGR_TYPE_WO;
    in.node = in.dst;
    in.module = m;
    in.object = CANOBJ_TARGET_RESET;
    in.data[0] = val;
    in.dlen = 1;
    if (lxcan_send (s, &in) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
}

void shutdown_request (void)
{
    struct canmgr_frame in;

    in.pri = 1;
    in.dst = n | 0x10;
    in.src = mynode;

    in.xpri = 1;
    in.type = CANMGR_TYPE_WO;
    in.node = in.dst;
    in.module = m;
    in.object = CANOBJ_TARGET_POWER;
    in.dlen = 1;
    in.data[0] = 2;
    if (lxcan_send (s, &in) < 0) {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
}

void canobj_consoleconn_ack (struct canmgr_frame *fr)
{
    if (fr->type == CANMGR_TYPE_NAK) {
        fprintf (stderr, "Got NAK to CONSOLECONN\n");
        exit (1);
    }
    fprintf (stderr, "Connected, type `&?' to list escapes.\n");
    new_tio = saved_tio;
    cfmakeraw(&new_tio);
    new_tio.c_cc[VMIN] = 1;
    new_tio.c_cc[VTIME] = 0;
    if (tcsetattr (0, TCSAFLUSH, &new_tio) < 0) {
        fprintf (stderr, "tcsetattr: %m\n");
        exit (1);
    }
    ev_io_start (loop, &stdin_watcher);
}

void canobj_consoledisc_ack (struct canmgr_frame *fr)
{
    if (fr->type == CANMGR_TYPE_NAK)
        fprintf (stderr, "Got NAK to CONSOLEDISC\n");
    ev_io_stop (loop, &can_watcher);
    if (tcsetattr (0, TCSAFLUSH, &saved_tio) < 0) {
        fprintf (stderr, "tcsetattr: %m\n");
        exit (1);
    }
}

void canobj_consolesend (struct canmgr_frame *fr)
{
    uint8_t *p;
    int rc, n = 0;
    switch (fr->type) {
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
            goto nak;
        case CANMGR_TYPE_DAT:
            do {
                if ((rc = write (STDOUT_FILENO, fr->data+n, fr->dlen-n)) < 0) {
                    fprintf (stderr, "stdout write error: %m\r\n");
                    break;
                }
                n += rc;
            } while (n < fr->dlen);
            canmgr_ack (fr, CANMGR_TYPE_ACK);
            break;
        default:
            break;
    }
    return;
nak:
    canmgr_ack (fr, CANMGR_TYPE_NAK);
}

void canobj_consolerecv_ack (struct canmgr_frame *fr)
{
    if (fr->type == CANMGR_TYPE_NAK)
        fprintf (stderr, "Got NAK to CONSOLERECV\n");
    expecting_recv_ack = 0;
    ev_io_start (loop, &stdin_watcher);
}

#define ISCRNL(c)   ((c)=='\n'||(c)=='\r')
static void stdin_cb (EV_P_ ev_io *w, int revents)
{
    static uint8_t hist[2] = "\r\r"; /* input history for escpes */
    struct canmgr_frame in;
    char c;
    uint8_t buf[8];
    int max = canmgr_maxdata (CANOBJ_TARGET_CONSOLERECV) - 1; // stored & char
    int i, len;

    in.pri = 1;
    in.dst = n | 0x10;
    in.src = mynode;

    in.xpri = 1;
    in.type = CANMGR_TYPE_DAT;
    in.node = in.dst;
    in.module = m;
    in.object = CANOBJ_TARGET_CONSOLERECV;

    if ((len = read (0, buf, max)) <= 0)
        return;
    in.dlen = 0;
    for (i = 0; i < len; i++) {
        if (ISCRNL (hist[0]) && hist[1] == '&') {
            int match = 1;
            switch (buf[i]) {
                case '?': // help
                    fprintf (stderr,
                            "Type `&' at beginning of a line followed by:\r\n");
                    fprintf (stderr, "    .   Exit cancon session\r\n");
                    fprintf (stderr, "    r   Dump ring buffer\r\n");
                    fprintf (stderr, "    ?   Print this help\r\n");
                    fprintf (stderr, "    I   Turn identify LED on\r\n");
                    fprintf (stderr, "    i   Turn identify LED off\r\n");
                    fprintf (stderr, "    R   Pulse hardware reset line\r\n");
                    fprintf (stderr, "    S   Press shutdown button\r\n");
                    break;
                case '.': // exit
                    console_request (CANOBJ_TARGET_CONSOLEDISC);
                    ev_io_stop (loop, &stdin_watcher);
                    break;
                case 'r': // ring buffer dump
                    console_request (CANOBJ_TARGET_CONSOLERING);
                    break;
                case 'i': // identify=off
                    identify_request (0);
                    break;
                case 'I': // identify=on
                    identify_request (1);
                    break;
                case 'R': // reset=3 (pulse)
                    reset_request (3);
                    break;
                case 'S': // shutdown
                    shutdown_request ();
                    break;
                default:
                    in.data[in.dlen++] = '&';
                    in.data[in.dlen++] = buf[i];
                    match = 0;
                    break;
            }
            if (match)
                buf[i] = '\r'; // allow back to back commands
        } else if (buf[i] != '&' || !ISCRNL (hist[1])) {
            in.data[in.dlen++] = buf[i];
        }
        hist[0] = hist[1];
        hist[1] = buf[i];
    }
    if (in.dlen == 0)
        return;
    if (lxcan_send (s, &in) < 0)  {
        fprintf (stderr, "lxcan_send: %m\n");
        exit (1);
    }
    expecting_recv_ack = 1;
    ev_io_stop (loop, &stdin_watcher);
}

static void can_cb (EV_P_ ev_io *w, int revents)
{
    struct canmgr_frame fr;

    if (lxcan_recv (s, &fr) < 0) {
        fprintf (stderr, "lxcan_recv: %m\n");
        return;
    }
    if (fr.dst != mynode)
        return; // not addressed to me
    if (fr.object == myobject) {
        canobj_consolesend (&fr);
        return;
    }
    if (fr.type != CANMGR_TYPE_ACK && fr.type != CANMGR_TYPE_NAK)
        return;

    switch (fr.object) {
        case CANOBJ_TARGET_CONSOLECONN:
            canobj_consoleconn_ack (&fr);
            break;
        case CANOBJ_TARGET_CONSOLEDISC:
            canobj_consoledisc_ack (&fr);
            break;
        case CANOBJ_TARGET_CONSOLERECV:
            canobj_consolerecv_ack (&fr);
            break;
        case CANOBJ_TARGET_CONSOLERING:
        case CANOBJ_LED_IDENTIFY:
        case CANOBJ_TARGET_RESET:
        case CANOBJ_TARGET_POWER:
            if (fr.type == CANMGR_TYPE_NAK)
                fprintf (stderr, "Got a NAK response to obj %d\n", fr.object);
            break;
    }
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

    // FIXME: object should be unique
    // Use CANOBJ_TARGET_CONSOLESEND for the common case,
    // but if that's in use, select from range CANOBJ_TARGET_CONSOLEBASE - 0xff

    if ((s = lxcan_open ("can0")) < 0) {
        fprintf (stderr, "lxcan_open: %m\n");
        exit (1);
    }
    if (tcgetattr (0, &saved_tio) < 0) {
        fprintf (stderr, "tcgetattr: %m\n");
        exit (1);
    }

    loop = EV_DEFAULT;
    ev_io_init (&can_watcher, can_cb, s, EV_READ);
    ev_io_init (&stdin_watcher, stdin_cb, 0, EV_READ);
    ev_io_start (loop, &can_watcher);

    console_request (CANOBJ_TARGET_CONSOLECONN);

    ev_run (loop, 0);

    lxcan_close (s);

    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
