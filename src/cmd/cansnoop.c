/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "src/libbramble/bramble.h"

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
    { CANMSG_OBJ_CONSOLECONN,    "CONSOLE-CONN" },
    { CANMSG_OBJ_CONSOLESEND,    "CONSOLE-SEND" },
    { CANMSG_OBJ_CONSOLERECV,    "CONSOLE-RECV" },
    { CANMSG_OBJ_CONSOLEDISC,    "CONSOLE-DISC" },
    { CANMSG_OBJ_SHUTDOWN,       "SHUTDOWN" },
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

static const char *canmsg_objstr (const struct canmsg *msg)
{
    return strtab_lookup (msg->object, objstr, sizeof (objstr));
}

static const char *canmsg_typestr (const struct canmsg *msg)
{
    return strtab_lookup (msg->type, typestr, sizeof (typestr));
}

int cansnoop_main (int argc, char *argv[])
{
    int fd;
    struct canmsg msg;
    double t_start = monotime ();
    char hex[32];
    char ascii[35];
    int i;
    char seq[8];

    if (argc != 1 && argc != 2)
        die ("Usage: bramble cansnoop [ADDR]\n");

    if (argc == 2) {
        char *endptr;
        int addr;

        errno = 0;
        addr = strtol (argv[1], &endptr, 16);
        if (errno != 0 || *endptr != '\0' || addr < 0 || addr > 0x3f)
            die ("error parsing hexadecimal ADDR from command line\n");

        struct can_filter rfilter[] = {
            {   .can_id = addr << CANMSG_DST_SHIFT,
                .can_mask = CANMSG_DST_MASK,
            },
            {   .can_id = addr << CANMSG_SRC_SHIFT,
                .can_mask = CANMSG_SRC_MASK,
            },
            {   .can_id = CANMSG_ADDR_BROADCAST << CANMSG_DST_SHIFT,
                .can_mask = CANMSG_DST_MASK,
            },
        };
        fd = can_open_with (BRAMBLE_CAN_INTERFACE, rfilter, 3);
    }
    else
        fd = can_open_with (BRAMBLE_CAN_INTERFACE, NULL, 0);

    if (fd < 0)
        die ("%s: %s\n", BRAMBLE_CAN_INTERFACE, strerror (errno));

    for (;;) {
        if (can_recv (fd, &msg) < 0)
            die ("can rx: %s\n", strerror (errno));
        hex[0] = '\0';
        for (i = 0; i < msg.dlen; i++) {
            snprintf (&hex[i*3],
                      4,
                      "%02x%s",
                      msg.data[i],
                      i == msg.dlen - 1 ? "" : " ");
            ascii[i] = isprint (msg.data[i]) ? msg.data[i] : '.';
        }
        seq[0] = '\0';
        if (msg.type == CANMSG_TYPE_DAT) {
            int n;
            snprintf (seq, sizeof (seq) - 1, "[%d]", msg.seq);
            n = strlen (seq);
            if (msg.eot)
                snprintf (seq + n, sizeof (seq) - n, "*");
        }

        printf ("%07.3f %.2x->%.2x  %s%-4s %-5s %-12s %-23s %s%.*s%s\n",
                monotime_since (t_start),
                msg.src,
                msg.dst,
                msg.pri == 0 ? "+" : " ",
                canmsg_typestr (&msg),
                seq,
                canmsg_objstr (&msg),
                hex,
                msg.dlen > 0 ? "`" : "",
                msg.dlen, ascii,
                msg.dlen > 0 ? "\'" : "");
    };

    close (fd);
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

