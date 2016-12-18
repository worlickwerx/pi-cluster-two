#include <sys/socket.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <linux/can.h>

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "canmgr_proto.h"

static const uint8_t myaddr = 0; // lie

int can_open (const char *name)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    int s;

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        fprintf (stderr, "socket: %m\n");
        return -1;
    }
    strcpy(ifr.ifr_name, "can0" );
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        fprintf (stderr, "ioctl SIOCGIFINDEX: %m\n");
        close (s);
        return -1;
    }
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind (s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf (stderr, "bind: %m\n");
        close (s);
        return -1;
    }
    return s;
}

int can_send (int s, struct rawcan_frame *raw)
{
    struct can_frame frame;

    frame.can_id = raw->id;
    frame.can_dlc = raw->dlen;
    memcpy (frame.data, raw->data, raw->dlen);

    if (write (s, &frame, sizeof(frame)) != sizeof (frame))
        return -1;
    return 0;
}

int can_recv (int s, struct rawcan_frame *raw)
{
    struct can_frame frame;

    if (read (s, &frame, sizeof(frame)) != sizeof (frame))
        return -1;

    raw->id = frame.can_id;
    raw->dlen = frame.can_dlc;
    memcpy (raw->data, frame.data, frame.can_dlc);
    return 0;
}

void can_close (int s)
{
    close (s);
}

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
    if ((s = can_open ("can0")) < 0)
        exit (1);
    if (can_send (s, &raw) < 0) {
        fprintf (stderr, "can_send: %m\n");
        exit (1);
    }
    // TODO timeout
    for (;;) {
        if (can_recv (s, &raw) < 0) {
            fprintf (stderr, "can_recv: %m\n");
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
