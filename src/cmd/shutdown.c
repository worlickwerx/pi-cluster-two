/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

/* shutdown.c - os friendly shutdown by slot
 */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <unistd.h>
#include <endian.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <argz.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>

#include "src/libbramble/bramble.h"

static int srcaddr = 0x2;
static int canfd;

static int parse_slot (const char *s)
{
    char *endptr;
    int slot;

    errno = 0;
    slot = strtol (s, &endptr, 10);
    if (errno != 0 || *endptr != '\0' || slot < 0 || slot > 16)
        return -1;
    return slot;
}

static int write_obj_empty (int slot, int object)
{
    int dstaddr = slot | CANMSG_ADDR_CONTROL;
    struct canobj *obj;

    if (!(obj = canobj_openfd (canfd, srcaddr, dstaddr, object))
        || canobj_write (obj, NULL, 0) < 0) {
        canobj_close (obj);
        return -1;
    }
    canobj_close (obj);
    return 0;
}

static int read_obj_byte (int slot, int object, uint8_t *val)
{
    int dstaddr = slot | CANMSG_ADDR_CONTROL;
    struct canobj *obj;
    uint8_t data[8];
    int n;

    if (!(obj = canobj_openfd (canfd, srcaddr, dstaddr, object))
        || (n = canobj_read (obj, data, sizeof (data))) < 0) {
        canobj_close (obj);
        return -1;
    }
    if (n != 1) {
        errno = EPROTO;
        canobj_close (obj);
        return -1;
    }
    *val = data[0];
    canobj_close (obj);
    return 0;
}

int shutdown_main (int argc, char *argv[])
{
    int myslot;
    int slot;
    uint8_t val;

    if (argc != 2)
        die ("Usage: bramble shutdown SLOT\n");

    if ((slot = parse_slot (argv[1])) < 0)
        die ("error parsing SLOT (0-15)\n");

    if ((myslot = slot_get ()) < 0)
        warn ("WARNING: failed to read slot: %s, assuming CAN address %x\n",
              strerror (errno), myslot);
    else
        srcaddr = myslot | CANMSG_ADDR_COMPUTE;

    if (slot == myslot) {
        warn ("WARNING: shutting down self, type ctrl-C with in 2s\n");
        sleep (2);
    }

    if ((canfd = can_open (BRAMBLE_CAN_INTERFACE, srcaddr)) < 0)
        die ("%s: %s\n", BRAMBLE_CAN_INTERFACE, strerror (errno));

    warn ("sending SHUTDOWN command\n");
    if (write_obj_empty (slot, CANMSG_OBJ_SHUTDOWN) < 0)
        die ("cannot write shutdown command to slot %d\n", slot);
    warn ("waiting for OS to stop running\n");
    do {
        if (read_obj_byte (slot, CANMSG_OBJ_SHUTDOWN, &val) < 0)
            die ("cannot read shutdown status from slot %d\n", slot);
        if (val == 0)
            usleep (1000*200);
    } while (val == 0);
    warn ("waiting for power to turn off\n");
    do {
        if (read_obj_byte (slot, CANMSG_OBJ_POWER, &val) < 0)
            die ("cannot read power status from slot %d\n", slot);
        if (val == 0)
            usleep (1000*200);
    } while (val == 1);

done:
    close (canfd);
    return 0;
}


/*
 * vi:ts=4 sw=4 expandtab
 */

