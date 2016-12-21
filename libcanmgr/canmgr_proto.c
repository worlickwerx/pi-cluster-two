#include <stdint.h>
#include <string.h>
#include "canmgr_proto.h"

int canmgr_decode (struct canmgr_frame *fr, struct rawcan_frame *raw)
{
    /* 2 bit object */
    fr->object = raw->id & 3;
    /* 6 bit node */
    fr->node = (raw->id>>2) & 0x3f;
    /* 6 bit module */
    fr->module = (raw->id>>8) & 0x3f;
    /* 3 bit type */
    fr->type = (raw->id>>14) & 7;
    /* 1 bit priority */
    fr->xpri = (raw->id>>17) & 1;
    /* 5 bit src */
    fr->src = (raw->id>>18) & 0x1f;
    /* 5 bit dst */
    fr->dst = (raw->id>>23) & 0x1f;
    /* 1 bit priority */
    fr->pri = (raw->id>>28) & 1;
    /* 0-8 bytes of data */
    if (fr->object == 3) { // extended object id
        if (raw->dlen == 0)
            return -1;
        fr->object += raw->data[0];
        fr->dlen = raw->dlen - 1;
        memcpy (fr->data, &raw->data[1], fr->dlen);
    } else {
        fr->dlen = raw->dlen;
        memcpy (fr->data, raw->data, fr->dlen);
    }
    return 0;
}

int canmgr_encode (struct canmgr_frame *fr, struct rawcan_frame *raw)
{
    if (fr->dlen > canmgr_maxdata (fr->object))
        return -1;

    /* 2 bit object */
    raw->id = fr->object > 3 ? 3 : fr->object;
    /* 6 bit node */
    raw->id |= fr->node<<2;
    /* 6 bit module */
    raw->id |= fr->module<<8;
    /* 3 bit type */
    raw->id |= fr->type<<14;
    /* 1 bit priority */
    raw->id |= fr->xpri<<17;
    /* 5 bit src */
    raw->id |= fr->src<<18;
    /* 5 bit dst */
    raw->id |= fr->dst<<23;
    /* 1 bit priority */
    raw->id |= fr->pri<<28;
    /* 0-8 bytes of data */
    if (fr->object >= 3) { // extended object id
        raw->data[0] = fr->object - 3;
        raw->dlen = fr->dlen + 1;
        memcpy (&raw->data[1], fr->data, fr->dlen);
    } else {
        raw->dlen = fr->dlen;
        memcpy (raw->data, fr->data, fr->dlen);
    }
    return 0;
}

int canmgr_maxdata (int object)
{
    return (object >= 3) ? 7 : 8;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

