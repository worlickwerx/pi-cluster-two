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

int opencan (const char *name)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    int s;

    if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
        fprintf (stderr, "socket: %m\n");
        exit (1);
    }
    strcpy(ifr.ifr_name, "can0" );
    if (ioctl(s, SIOCGIFINDEX, &ifr) < 0) {
        fprintf (stderr, "ioctl SIOCGIFINDEX: %m\n");
        exit (1);
    }
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf (stderr, "bind: %m\n");
        exit (1);
    }
    return s;
}

int main (int argc, char *argv[])
{
    struct canmgr_pkt req_pkt, rep_pkt;
    struct canmgr_id req_id, rep_id;
    int req_dlc, rep_dlc;
    int c, m, n;
    int s;
    struct can_frame frame;

    if (argc != 3) {
        fprintf (stderr, "Usage: canpower c,m,n 0|1\n");
        exit (1);
    }
    if (sscanf (argv[1], "%d,%d,%d", &c, &m, &n) != 3
            || c < 0 || c >= 0x10 || m < 0 || m >= 0x10 || c < 0 || c >= 0x10) {
        fprintf (stderr, "improperly specified target\n");
        exit (1);
    }
    /* construct canmgr pkt and id
     * for now, no routing - cluster and module are ignored
     */
    req_pkt.pri = 1;
    req_pkt.type = CANMGR_TYPE_WO;
    req_pkt.node = n | 0x10;
    req_pkt.module = m;
    req_pkt.cluster = c;
    req_pkt.object = CANOBJ_TARGET_POWER;
    req_pkt.data[0] = strtoul (argv[2], NULL, 10); /* 0=off, 1=on */
    req_dlc = 1;

    req_id.pri = 1;
    req_id.dst = req_pkt.node;
    req_id.src = myaddr;

    /* construct linux can frame
     */
    frame.can_id = req_id.src;
    frame.can_id |= (req_id.dst<<5);
    frame.can_id |= (req_id.pri<<10);
    if ((frame.can_dlc = canmgr_pkt_encode (&req_pkt,
                                            req_dlc, &frame.data[0], 8)) < 0) {
        fprintf (stderr, "error encoding CAN packet\n");
        exit (1);
    }

    /* send frame on can0
     * wait for ack/nak response
     */
    s = opencan ("can0");
    if (write(s, &frame, sizeof(frame)) != sizeof (frame)) {
        fprintf (stderr, "write: %m\n");
        exit (1);
    }
    // TODO timeout
    for (;;) {
        if (read(s, &frame, sizeof(frame)) != sizeof (frame)) {
            fprintf (stderr, "read: %m\n");
            exit (1);
        }
        rep_id.src = frame.can_id & 0x1f;
        rep_id.dst = (frame.can_id>>5) & 0x1f;
        rep_id.pri = (frame.can_id>>10) & 1;
        rep_dlc = canmgr_pkt_decode (&rep_pkt, &frame.data[0], frame.can_dlc);
        if (rep_dlc < 0) {
            fprintf (stderr, "canmgr_pkt_decode error, ignoring pkt\n");
            continue;
        }
        if (rep_id.src != req_id.dst || rep_id.dst != req_id.src)
            continue;
        if (rep_pkt.object != CANOBJ_TARGET_POWER)
            continue;
        if (rep_pkt.cluster != req_pkt.cluster
                || rep_pkt.module != req_pkt.module
                || rep_pkt.node != req_pkt.node)
            continue;
        if (rep_pkt.type == CANMGR_TYPE_ACK) {
            fprintf (stderr, "command accepted\n");
            exit (0);
        }
        if (rep_pkt.type == CANMGR_TYPE_NAK) {
            fprintf (stderr, "command failed\n");
            exit (1);
        }
    }
    
    close (s);

    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
