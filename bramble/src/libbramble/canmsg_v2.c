/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>


#include "canmsg.h"
#include "canmsg_v2.h"

struct strtab {
    int id;
    const char *name;
};

static struct strtab typestr_v2[] = {
    { CANMSG_V2_TYPE_RO,    "RO" },
    { CANMSG_V2_TYPE_WO,    "WO" },
    { CANMSG_V2_TYPE_WNA,   "WNA" },
    { CANMSG_V2_TYPE_DAT,   "D" },
    { CANMSG_V2_TYPE_ACK,   "ACK" },
    { CANMSG_V2_TYPE_NAK,   "NAK" },
    { CANMSG_V2_TYPE_SIG,   "SIG" },
};

static struct strtab objstr_v2[] = {
    { CANMSG_V2_OBJ_ECHO,           "ECHO" },
    { CANMSG_V2_OBJ_POWER,          "POWER" },
    { CANMSG_V2_OBJ_POWER_MEASURE,  "POWER-MEASURE" },
    { CANMSG_V2_OBJ_CONSOLECONN,    "CONSOLE-CONN" },
    { CANMSG_V2_OBJ_CONSOLESEND,    "CONSOLE-SEND" },
    { CANMSG_V2_OBJ_CONSOLERECV,    "CONSOLE-RECV" },
    { CANMSG_V2_OBJ_CONSOLEDISC,    "CONSOLE-DISC" },
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

const char *canmsg_v2_objstr (const struct canmsg_v2 *msg)
{
    return strtab_lookup (msg->object, objstr_v2, sizeof (objstr_v2));
}

const char *canmsg_v2_typestr (const struct canmsg_v2 *msg)
{
    return strtab_lookup (msg->type, typestr_v2, sizeof (typestr_v2));
}

int canmsg_v2_decode (const struct canmsg_raw *raw, struct canmsg_v2 *msg)
{
    if (!raw->xmsgidf || raw->length > 8)
        return -1;
    msg->seq = raw->msgid & 0x1f;
    msg->object = (raw->msgid>>5) & 0xff;
    msg->type = (raw->msgid>>13) & 7;
    msg->src = (raw->msgid>>16) & 0x3f;
    msg->dst = (raw->msgid>>22) & 0x3f;
    msg->pri = (raw->msgid>>28) & 1;
    msg->dlen = raw->length;
    memcpy (msg->data, raw->data, msg->dlen);
    return 0;
}

int canmsg_v2_encode (const struct canmsg_v2 *msg, struct canmsg_raw *raw)
{
    raw->msgid = msg->seq;
    raw->msgid |= msg->object<<5;
    raw->msgid |= msg->type<<13;
    raw->msgid |= msg->src<<16;
    raw->msgid |= msg->dst<<22;
    raw->msgid |= msg->pri<<28;
    raw->length = msg->dlen;
    memcpy (raw->data, msg->data, msg->dlen);
    raw->xmsgidf = true;
    raw->rtrf = false;
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
