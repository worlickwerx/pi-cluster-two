/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>


#include "canmsg_v2.h"

struct strtab {
    int id;
    const char *name;
};

static struct strtab typestr[] = {
    { CANMSG_TYPE_RO,    "RO" },
    { CANMSG_TYPE_WO,    "WO" },
    { CANMSG_TYPE_WNA,   "WNA" },
    { CANMSG_TYPE_DAT,   "D" },
    { CANMSG_TYPE_ACK,   "ACK" },
    { CANMSG_TYPE_NAK,   "NAK" },
    { CANMSG_TYPE_SIG,   "SIG" },
};

static struct strtab objstr[] = {
    { CANMSG_OBJ_ECHO,           "ECHO" },
    { CANMSG_OBJ_POWER,          "POWER" },
    { CANMSG_OBJ_POWER_MEASURE,  "POWER-MEASURE" },
    { CANMSG_OBJ_CONSOLECONN,    "CONSOLE-CONN" },
    { CANMSG_OBJ_CONSOLESEND,    "CONSOLE-SEND" },
    { CANMSG_OBJ_CONSOLERECV,    "CONSOLE-RECV" },
    { CANMSG_OBJ_CONSOLEDISC,    "CONSOLE-DISC" },
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

const char *canmsg_objstr (const struct canmsg *msg)
{
    return strtab_lookup (msg->object, objstr, sizeof (objstr));
}

const char *canmsg_typestr (const struct canmsg *msg)
{
    return strtab_lookup (msg->type, typestr, sizeof (typestr));
}

/*
 * vi:ts=4 sw=4 expandtab
 */
