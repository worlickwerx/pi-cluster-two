/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <sys/ioctl.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <poll.h>
#include <net/if.h>
#include <linux/sockios.h>
#include <string.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>

#include "canlinux.h"

int can_open_with (const char *name, const struct can_filter *rfilter, int len)
{
    struct sockaddr_can addr;
    struct ifreq ifr;
    int fd;
    int saved_errno;

    if ((fd = socket (PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
        return -1;
    strcpy (ifr.ifr_name, name);
    if (ioctl (fd, SIOCGIFINDEX, &ifr) < 0)
        goto error;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    if (bind (fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        goto error;
    if (rfilter && len > 0) {
        if (setsockopt (fd,
                        SOL_CAN_RAW,
                        CAN_RAW_FILTER,
                        rfilter,
                        sizeof (*rfilter) * len) < 0)
            goto error;
    }
    return fd;
error:
    saved_errno = errno;
    close (fd);
    errno = saved_errno;
    return -1;
}

static bool valid_linux_address (int addr)
{
    if (addr == CANMSG_ADDR_MANAGEMENT)
        return true;
    if (addr >= CANMSG_ADDR_COMPUTE && addr <= (CANMSG_ADDR_COMPUTE | 0x0f))
        return true;
    errno = EINVAL;
    return false;
}

int can_open (const char *name, int myaddr)
{
    if (!valid_linux_address (myaddr))
        return -1;

    struct can_filter rfilter[] = {
        {   .can_id = myaddr << CANMSG_DST_SHIFT,
            .can_mask = CANMSG_DST_MASK,
        },
        {   .can_id = CANMSG_ADDR_BROADCAST << CANMSG_DST_SHIFT,
            .can_mask = CANMSG_DST_MASK,
        },
    };
    return can_open_with (name, rfilter, 2);
}

int can_recv (int fd, struct canmsg *msg)
{
    struct can_frame fr;
    int n;
    uint32_t id;

    if ((n = read (fd, &fr, sizeof (fr))) < 0)
        return -1;
    if (n != sizeof (fr) || fr.can_dlc > 8)
        goto eproto;
    if (!(fr.can_id & CAN_EFF_FLAG))
        goto eproto;

    id = fr.can_id & CAN_EFF_MASK;
    msg->seq = id & 0x1f;
    msg->object = (id>>5) & 0xff;
    msg->type = (id>>13) & 7;
    msg->src = (id>>16) & 0x3f;
    msg->dst = (id>>22) & 0x3f;
    msg->pri = (id>>28) & 1;

    memcpy (msg->data, fr.data, fr.can_dlc);
    msg->dlen = fr.can_dlc;
    return 0;
eproto:
    errno = EPROTO;
    return -1;
}

int can_recv_timeout (int fd, struct canmsg *msg, double timeout)
{
    struct pollfd pfd = { .fd = fd, .events = POLLIN };
    int rc;

    if ((rc = poll (&pfd, 1, timeout * 1E3)) < 0)
        return -1;
    if (rc == 0) {
        errno = ETIMEDOUT;
        return -1;
    }
    return can_recv (fd, msg);
}

int can_send (int fd, struct canmsg *msg)
{
    struct can_frame fr;
    int n;

    fr.can_id = msg->seq;
    fr.can_id |= msg->object<<5;
    fr.can_id |= msg->type<<13;
    fr.can_id |= msg->src<<16;
    fr.can_id |= msg->dst<<22;
    fr.can_id |= msg->pri<<28;
    fr.can_id |= CAN_EFF_FLAG;

    fr.can_dlc = msg->dlen;
    memcpy (fr.data, msg->data, msg->dlen);

    if ((n = write (fd, &fr, sizeof (fr))) < 0)
        return -1;
    if (n != sizeof (fr))
        goto eproto; // should not happen
    return 0;
eproto:
    errno = EPROTO;
    return -1;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
