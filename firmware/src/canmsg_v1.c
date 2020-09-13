/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "FreeRTOS.h"
#include "string.h"

#include "canmsg.h"
#include "canmsg_v1.h"
#include "trace.h"

struct strtab {
    int id;
    const char *name;
};

struct strtab typestr_v1[] = {
    { CANMSG_V1_TYPE_RO,    "RO" },
    { CANMSG_V1_TYPE_WO,    "WO" },
    { CANMSG_V1_TYPE_WNA,   "WNA" },
    { CANMSG_V1_TYPE_DAT,   "D" },
    { CANMSG_V1_TYPE_ACK,   "ACK" },
    { CANMSG_V1_TYPE_NAK,   "NAK" },
    { CANMSG_V1_TYPE_SIG,   "SIG" },
};

struct strtab objstr_v1[] = {
    { CANMSG_V1_OBJ_HEARTBEAT,      "HB" },
    { CANMSG_V1_OBJ_CONSOLECONN,    "CONSOLECONN" },
    { CANMSG_V1_OBJ_CONSOLEDISC,    "CONSOLEDISC" },
    { CANMSG_V1_OBJ_CONSOLESEND,    "CONSOLESEND" },
    { CANMSG_V1_OBJ_CONSOLERECV,    "CONSOLERECV" },
    { CANMSG_V1_OBJ_CONSOLERING,    "CONSOLERING" },
    { CANMSG_V1_OBJ_POWER,          "POWER" },
    { CANMSG_V1_OBJ_RESET,          "RESET" },
    { CANMSG_V1_OBJ_ECHO,           "ECHO" },
    { CANMSG_V1_OBJ_LED_IDENTIFY,   "IDENTIFY" },
};

static const char *strtab_lookup (int id, const struct strtab *tab, size_t size)
{
    uint8_t i;
    for (i = 0; i < size / sizeof (struct strtab); i++) {
        if (tab[i].id == id)
            return tab[i].name;
    }
    return "?";
}

void canmsg_v1_trace (const struct canmsg_v1 *msg)
{
    trace_printf ("%x->%x %s %s [%d bytes]\n",
                  msg->src,
                  msg->dst,
                  strtab_lookup (msg->type, typestr_v1, sizeof (typestr_v1)),
                  strtab_lookup (msg->object, objstr_v1, sizeof (objstr_v1)),
                  msg->dlen);
}

int canmsg_v1_decode (const struct canmsg_raw *raw, struct canmsg_v1 *msg)
{
    if (!raw->xmsgidf)
        return -1;
    msg->object = raw->msgid & 3;
    msg->node = (raw->msgid>>2) & 0x3f;
    msg->module = (raw->msgid>>8) & 0x3f;
    msg->type = (raw->msgid>>14) & 7;
    msg->xpri = (raw->msgid>>17) & 1;
    msg->src = (raw->msgid>>18) & 0x1f;
    msg->dst = (raw->msgid>>23) & 0x1f;
    msg->pri = (raw->msgid>>28) & 1;
    if (msg->object == 3) { // extended object id
        if (raw->length == 0)
            return -1;
        msg->object += raw->data[0];
        msg->dlen = raw->length - 1;
        memcpy (msg->data, &raw->data[1], msg->dlen);
    } else {
        msg->dlen = raw->length;
        memcpy (msg->data, raw->data, msg->dlen);
    }
    return 0;
}

int canmsg_v1_encode (const struct canmsg_v1 *msg, struct canmsg_raw *raw)
{
    int maxdata = msg->object >= 3 ? 7 : 8;

   if (msg->dlen > maxdata)
        return -1;
    raw->msgid = msg->object > 3 ? 3 : msg->object;
    raw->msgid |= msg->node<<2;
    raw->msgid |= msg->module<<8;
    raw->msgid |= msg->type<<14;
    raw->msgid |= msg->xpri<<17;
    raw->msgid |= msg->src<<18;
    raw->msgid |= msg->dst<<23;
    raw->msgid |= msg->pri<<28;
    if (msg->object >= 3) { // extended object id
        raw->data[0] = msg->object - 3;
        raw->length = msg->dlen + 1;
        memcpy (&raw->data[1], msg->data, msg->dlen);
    } else {
        raw->length = msg->dlen;
        memcpy (raw->data, msg->data, msg->dlen);
    }
    raw->fmi = 0;
    raw->xmsgidf = 1;
    raw->rtrf = 0;
    raw->fifo = 0;
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
