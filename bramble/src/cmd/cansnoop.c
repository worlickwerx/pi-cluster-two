/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>

#include "src/libbramble/bramble.h"

static double monotime (void)
{
    struct timespec ts;

    clock_gettime (CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1E-9;
}

static int usage (void)
{
    fprintf (stderr, "Usage: bramble cansnoop\n");
    return 1;
}

int cansnoop_main (int argc, char *argva[])
{
    int fd;
    struct canmsg_raw raw;
    struct canmsg_v1 msg;
    double t_start = monotime ();
    char hex[32];
    char ascii[35];
    int i;

    if (argc != 1)
        return usage ();
    fd = can_open (BRAMBLE_CAN_INTERFACE);
    if (fd < 0) {
        fprintf (stderr, "%s: %s", BRAMBLE_CAN_INTERFACE, strerror (errno));
        return 1;
    }
    while (can_recv (fd, &raw) == 0) {
        if (canmsg_v1_decode (&raw, &msg) < 0) {
            fprintf (stderr, "could not decode canmsg_v1 frame\n");
            continue;
        }
        hex[0] = '\0';
        for (i = 0; i < msg.dlen; i++) {
            snprintf (&hex[i*3],
                      4,
                      "%02x%s",
                      msg.data[i],
                      i == msg.dlen - 1 ? "" : " ");
            ascii[i] = isprint (msg.data[i]) ? msg.data[i] : '.';
        }

        printf ("%07.3f %.3x->%.3x  %-4s %-12s %-23s %s%.*s%s\n",
                monotime () - t_start,
                msg.src,
                msg.dst,
                canmsg_v1_typestr (&msg),
                canmsg_v1_objstr (&msg),
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

