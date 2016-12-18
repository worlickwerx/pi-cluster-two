#include <stdint.h>
#include <string.h>
#include "canmgr_proto.h"

int canmgr_compare_hdr (struct canmgr_hdr *h1, struct canmgr_hdr *h2)
{
    if (h1->object != h2->object || h1->node != h2->node
                                 || h1->module != h2->module
                                 || h1->cluster != h2->cluster
                                 || h1->type != h2->type
                                 || h1->pri != h2->pri)
        return -1;
    return 0;
}

int canmgr_decode_hdr (struct canmgr_hdr *hdr, uint8_t *data, int len)
{
    if (len != 4)
        return -1;

    /* 10 bit object id (8 + 2) */
    hdr->object = data[3];
    hdr->object |= (data[2]&3)<<8;
    /* 6 bit node */
    hdr->node = data[2]>>2;
    /* 6 bit module */
    hdr->module = data[1]&0x3f;
    /* 6 bit cluster (2 + 4) */
    hdr->cluster = data[1]>>6;
    hdr->cluster |= (data[0]&0x0f)<<2;
    /* 3 bit type */
    hdr->type = (data[0]&0x70)>>4;
    /* 1 bit priority */
    hdr->pri = (data[0]&0x80)>>7;

    return 0;
}

int canmgr_decode (struct canmgr_frame *fr, struct rawcan_frame *raw)
{
    if (raw->dlen < 4 || raw->dlen > 8)
        return -1;

    /* 11 bit CAN id */
    fr->id.src = raw->id & 0x1f;
    fr->id.dst = (raw->id>>5) & 0x1f;
    fr->id.pri = (raw->id>>10) & 1;

    if (canmgr_decode_hdr (&fr->hdr, &raw->data[0], 4) < 0)
        return -1;

    /* 0-4 bytes of data */
    fr->dlen = raw->dlen - 4;
    memcpy (&fr->data[0], &raw->data[4], fr->dlen);

    return 0;
}

int canmgr_encode_hdr (struct canmgr_hdr *hdr, uint8_t *data, int len)
{
    if (len != 4)
        return 0;

    /* 10 bit object id (8 + 2) */
    data[3] = hdr->object&0xff;
    data[2] = (hdr->object&0x300)>>8;
    /* 6 bit node */
    data[2] |= (hdr->node<<2);
    /* 6 bit module */
    data[1] = hdr->module;
    /* 6 bit cluster (2 + 4) */
    data[1] |= (hdr->cluster&3)<<6;
    data[0] = hdr->cluster>>2;
    /* 3 bit type */
    data[0] |= hdr->type<<4;
    /* 1 bit priority */
    data[0] |= hdr->pri<<7;

    return 0;
}

int canmgr_encode (struct canmgr_frame *fr, struct rawcan_frame *raw)
{
    if (fr->dlen > 4)
        return -1;

    raw->id = fr->id.src;
    raw->id |= fr->id.dst<<5;
    raw->id |= fr->id.pri<<10;

    if (canmgr_encode_hdr (&fr->hdr, &raw->data[0], 4) < 0)
        return -1;

    /* 0-4 bytes of data */
    raw->dlen = fr->dlen + 4;
    memcpy (&raw->data[4], &fr->data[0], fr->dlen);

    return 0;
}


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

