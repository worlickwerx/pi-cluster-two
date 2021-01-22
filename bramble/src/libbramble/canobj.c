/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <unistd.h>
#include <poll.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "bramble.h"

#define DEFAULT_TIMEOUT 0.5

struct canobj {
    int     canfd;
    int     src;
    int     dst;
    int     objid;
    double  tmout;
    bool    ownfd;
};

void canobj_close (struct canobj *obj)
{
    if (obj) {
        int saved_errno = errno;
        if (obj->canfd >= 0 && obj->ownfd)
            close (obj->canfd);
        free (obj);
        errno = saved_errno;
    }
}

struct canobj *canobj_open_with (int canfd, int sslot, int dslot, int objid)
{
    struct canobj *obj;

    if (!(obj = calloc (1, sizeof (*obj))))
        return NULL;
    obj->canfd = canfd;
    obj->src = sslot | 0x10;
    obj->dst = dslot;
    obj->objid = objid;
    obj->tmout = DEFAULT_TIMEOUT; // hardwired for now
    return obj;
}

struct canobj *canobj_open (int dslot, int objid)
{
    struct canobj *obj;
    int canfd;
    int sslot;

    if ((sslot = slot_get ()) < 0)
        return NULL;
    if ((canfd = can_open (BRAMBLE_CAN_INTERFACE)) < 0)
        return NULL;
    if (!(obj = canobj_open_with (canfd, sslot, dslot, objid)))
        goto error;
    obj->ownfd = true;
    return obj;
error:
    close (canfd);
    return NULL;
}


int canobj_write (struct canobj *obj, void *data, int len)
{
    struct canmsg_v1 msg = { 0 };
    struct canmsg_raw raw;
    double t_start = monotime ();

    if (len < 0 || len > 8) {
        errno = EINVAL;
        return -1;
    }
    /* Send "write object" message.
     */
    msg.pri = 1;
    msg.src = obj->src;
    msg.dst = obj->dst;
    msg.xpri = 1;
    msg.type = CANMSG_V1_TYPE_WO;
    msg.object = obj->objid;
    msg.dlen = len;
    memcpy (msg.data, data, len);
    if (canmsg_v1_encode (&msg, &raw) < 0)
        return -1;
    if (can_send (obj->canfd, &raw) < 0)
        return -1;

    /* Receive messages with timeout, filtering non-matched.
     */
    do {
        double timeleft = obj->tmout - monotime_since (t_start);
        if (timeleft < 0 && obj->tmout >= 0)
            timeleft = 0;
        if (can_recv_timeout (obj->canfd, &raw, timeleft) < 0)
            return -1;
        if (canmsg_v1_decode (&raw, &msg) < 0)
            return -1;
    } while (msg.dst != obj->src || msg.src != obj->dst
                                 || msg.object != obj->objid);

    /* Message matched.
     * Handle success/failure of write object operation.
     */
    switch (msg.type) {
        case CANMSG_V1_TYPE_ACK:
            break;
        case CANMSG_V1_TYPE_NAK:
            errno = ENOTSUP;
            return -1;
        default:
            errno = EPROTO;
            return -1;
    }
    return 0;
}


int canobj_read (struct canobj *obj, void *data, int len)
{
    struct canmsg_v1 msg = { 0 };
    struct canmsg_raw raw;
    double t_start = monotime ();

    if (len < 0 || len > 8) {
        errno = EINVAL;
        return -1;
    }
    /* Send "read object" message.
     */
    msg.pri = 1;
    msg.src = obj->src;
    msg.dst = obj->dst;
    msg.xpri = 1;
    msg.type = CANMSG_V1_TYPE_RO;
    msg.object = obj->objid;
    if (canmsg_v1_encode (&msg, &raw) < 0)
        return -1;
    if (can_send (obj->canfd, &raw) < 0)
        return -1;

    /* Receive messages with timeout, filtering non-matched.
     */
    do {
        double timeleft = obj->tmout - monotime_since (t_start);
        if (timeleft < 0 && obj->tmout >= 0)
            timeleft = 0;
        if (can_recv_timeout (obj->canfd, &raw, timeleft) < 0)
            return -1;
        if (canmsg_v1_decode (&raw, &msg) < 0)
            return -1;
    } while (msg.dst != obj->src || msg.src != obj->dst
                                 || msg.object != obj->objid);

    /* Message matched.
     * Handle success/failure of read object operation.
     */
    switch (msg.type) {
        case CANMSG_V1_TYPE_ACK:
            if (msg.dlen > len) {
                errno = EPROTO;
                return -1;
            }
            memcpy (data, msg.data, msg.dlen);
            break;
        case CANMSG_V1_TYPE_NAK:
            errno = ENOTSUP;
            return -1;
        default:
            errno = EPROTO;
            return -1;
    }
    return msg.dlen;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
