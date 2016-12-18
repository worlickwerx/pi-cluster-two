#include <stdint.h>
#include <string.h>
#include "canmgr_proto.h"

int canmgr_decode (struct canmgr_frame *fr, struct rawcan_frame *raw)
{
    if (raw->dlen < 4 || raw->dlen > 8)
        return -1;

    /* 11 bit CAN id */
    fr->id.src = raw->id & 0x1f;
    fr->id.dst = (raw->id>>5) & 0x1f;
    fr->id.pri = (raw->id>>10) & 1;

    /* 10 bit object id (8 + 2) */
    fr->hdr.object = raw->data[3];
    fr->hdr.object |= (raw->data[2]&3)<<8;
    /* 6 bit node */
    fr->hdr.node = raw->data[2]>>2;
    /* 6 bit module */
    fr->hdr.module = raw->data[1]&0x3f;
    /* 6 bit cluster (2 + 4) */
    fr->hdr.cluster = raw->data[1]>>6;
    fr->hdr.cluster |= (raw->data[0]&0x0f)<<2;
    /* 3 bit type */
    fr->hdr.type = (raw->data[0]&0x70)>>4;
    /* 1 bit priority */
    fr->hdr.pri = (raw->data[0]&0x80)>>7;

    /* 0-4 bytes of data */
    fr->dlen = raw->dlen - 4;
    memcpy (&fr->data[0], &raw->data[4], fr->dlen);

    return 0;
}

int canmgr_encode (struct canmgr_frame *fr, struct rawcan_frame *raw)
{
    if (fr->dlen > 4)
        return -1;

    raw->id = fr->id.src;
    raw->id |= fr->id.dst<<5;
    raw->id |= fr->id.pri<<10;

    /* 10 bit object id (8 + 2) */
    raw->data[3] = fr->hdr.object&0xff;
    raw->data[2] = (fr->hdr.object&0x300)>>8;
    /* 6 bit node */
    raw->data[2] |= (fr->hdr.node<<2);
    /* 6 bit module */
    raw->data[1] = fr->hdr.module;
    /* 6 bit cluster (2 + 4) */
    raw->data[1] |= (fr->hdr.cluster&3)<<6;
    raw->data[0] = fr->hdr.cluster>>2;
    /* 3 bit type */
    raw->data[0] |= fr->hdr.type<<4;
    /* 1 bit priority */
    raw->data[0] |= fr->hdr.pri<<7;

    /* 0-4 bytes of data */
    raw->dlen = fr->dlen + 4;
    memcpy (&raw->data[4], &fr->data[0], fr->dlen);

    return 0;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

