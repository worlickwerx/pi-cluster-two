/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>

#include "src/libbramble/bramble.h"

int cansnoop_main (int argc, char *argv[])
{
    int fd;
    struct canmsg msg;
    double t_start = monotime ();
    char hex[32];
    char ascii[35];
    int i;

    if (argc != 1)
        die ("Usage: bramble cansnoop\n");
    if ((fd = can_open (BRAMBLE_CAN_INTERFACE)) < 0)
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

        printf ("%07.3f %.2x->%.2x  %-4s %-12s %-23s %s%.*s%s\n",
                monotime_since (t_start),
                msg.src,
                msg.dst,
                canmsg_typestr (&msg),
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

