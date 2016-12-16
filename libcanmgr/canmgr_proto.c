#include <stdint.h>
#include <string.h>
#include "canmgr_proto.h"

int canmgr_pkt_decode (struct canmgr_pkt *pkt, uint8_t *buf, int buf_len)
{
    int data_len = buf_len - 4;
    if (buf_len < 4)
	    return -1; // runt
    /* 10 bit object id (8 + 2) */
    pkt->object = buf[3];
    pkt->object |= (buf[2]&3)<<8;
    /* 6 bit node */
    pkt->node = buf[2]>>2;
    /* 6 bit module */
    pkt->module = buf[1]&0x3f;
    /* 6 bit cluster (2 + 4) */
    pkt->cluster = buf[1]>>6;
    pkt->cluster |= (buf[0]&0x0f)<<2;
    /* 3 bit type */
    pkt->type = (buf[0]&0x70)>>4;
    /* 1 bit priority */
    pkt->pri = (buf[0]&0x80)>>7;
    /* 0-4 bytes of data */
    memcpy (pkt->data, &buf[4], data_len);

    return data_len;
}

int canmgr_pkt_encode (struct canmgr_pkt *pkt, int data_len,
                       uint8_t *buf, int buf_len)
{
    if (data_len < 0 || data_len > 4)
        return -1;
    if (buf_len < 4 + data_len)
        return -1;
    /* 10 bit object id (8 + 2) */
    buf[3] = pkt->object&0xff;
    buf[2] = (pkt->object&0x300)>>8;
    /* 6 bit node */
    buf[2] |= (pkt->node<<2);
    /* 6 bit module */
    buf[1] = pkt->module;
    /* 6 bit cluster (2 + 4) */
    buf[1] |= (pkt->cluster&3)<<6;
    buf[0] = pkt->cluster>>2;
    /* 3 bit type */
    buf[0] |= pkt->type<<4;
    /* 1 bit priority */
    buf[0] |= pkt->pri<<7;
    /* 0-4 bytes of data */
    memcpy (&buf[4], pkt->data, data_len);

    return buf_len - 4 + data_len;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

