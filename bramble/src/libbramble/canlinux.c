/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <string.h>
#include <linux/can.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "canlinux.h"

int can_open (const char *name)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    int fd;

    if ((fd = socket (PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
        return -1;
    strcpy (ifr.ifr_name, name);
    if (ioctl (fd, SIOCGIFINDEX, &ifr) < 0) {
        close (fd);
        return -1;
    }
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind (fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close (fd);
        return -1;
    }
    return fd;
}

int can_recv (int fd, struct canmsg_raw *raw)
{
    struct can_frame fr;
    int n;

    n = read (fd, &fr, sizeof (fr));
    if (n < 0)
        return -1;
    if (n != sizeof (fr) || fr.can_dlc > 8) {
        errno = EPROTO;
        return -1;
    }
    raw->length = fr.can_dlc;
    if ((fr.can_id & CAN_EFF_FLAG)) {
        raw->msgid = fr.can_id & CAN_EFF_MASK;
        raw->xmsgidf = true;
    }
    else {
        raw->msgid = fr.can_id & CAN_SFF_MASK;
        raw->xmsgidf = true;
    }
    if ((fr.can_id & CAN_RTR_FLAG))
        raw->rtrf = true;
    else
        raw->rtrf = false;
    memcpy (raw->data, fr.data, fr.can_dlc);
    return 0;
}

int can_send (int fd, struct canmsg_raw *raw)
{
    struct can_frame fr;
    int n;

    fr.can_dlc = raw->length;
    fr.can_id = raw->msgid;
    if (raw->xmsgidf)
        fr.can_id |= CAN_EFF_FLAG;
    if (raw->rtrf)
        fr.can_id |= CAN_RTR_FLAG;
    memcpy (fr.data, raw->data, raw->length);
    n = write (fd, &fr, sizeof (fr));
    if (n < 0)
        return -1;
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
