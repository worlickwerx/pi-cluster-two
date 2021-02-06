/* SPDX-License-Identifier: GPL-3.0-or-later */

/* power.c - helper for powerman
 *
 * This program is intended be spawned by powerman, e.g. in powerman.conf:
 *
 *   include "/usr/local/etc/powerman/bramble.dev"
 *   device "picl" "bramble" "/usr/local/bin/bramble powerman-helper |&"
 *   node "picl[0-7]" "picl" "[0-7]"
 *
 * Ensure that the user running powermand (e.g. daemon) is in the i2c
 * group so that this program can read the local slot.
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

static int srcaddr;
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

static void cmd_help (void)
{
    printf ("Commands: on, off, status, measure\n");
}

static int write_power (int slot, int val)
{
    int dstaddr = slot | CANMSG_ADDR_CONTROL;
    struct canobj *obj;
    uint8_t data[1] = { val };

    if (!(obj = canobj_openfd (canfd, srcaddr, dstaddr, CANMSG_OBJ_POWER))
             || canobj_write (obj, data, sizeof (data)) < 0)
        printf ("%d: %s\n", slot, strerror (errno));
    else
        printf ("OK\n");
    canobj_close (obj);
}

static int read_power (int slot)
{
    int dstaddr = slot | CANMSG_ADDR_CONTROL;
    struct canobj *obj;
    uint8_t data[8];
    int n;

    if (!(obj = canobj_openfd (canfd, srcaddr, dstaddr, CANMSG_OBJ_POWER))
        || (n = canobj_read (obj, data, sizeof (data))) < 0)
        printf ("%d: %s\n", slot, strerror (errno));
    else {
        printf ("%d: %s\n", slot, data[0] ? "on" : "off");
    }
    canobj_close (obj);
}


static int read_power_measure (int slot)
{
    int dstaddr = slot | CANMSG_ADDR_CONTROL;
    struct canobj *obj;
    uint8_t data[8];
    int n;

    if (!(obj = canobj_openfd (canfd, srcaddr, dstaddr,
                               CANMSG_OBJ_POWER_MEASURE))
        || (n = canobj_read (obj, data, sizeof (data))) < 0)
        printf ("%d: %s\n", slot, strerror (errno));
    else {
        uint16_t ma = be16toh (*(uint16_t *)&data[0]);
        printf ("%d: %.3fA\n", slot, 1E-3*ma);
    }
    canobj_close (obj);
}

static void cmd_on (int argc, char **argv)
{
    int slot;

    if (argc != 2 || (slot = parse_slot (argv[1])) < 0) {
        warn ("Usage: on SLOT (0-15)\n");
        return;
    }
    write_power (slot, 1);
}

static void cmd_off (int argc, char **argv)
{
    int slot;

    if (argc != 2 || (slot = parse_slot (argv[1])) < 0) {
        warn ("Usage: on SLOT (0-15)\n");
        return;
    }
    write_power (slot, 0);
}

static void cmd_status (int argc, char **argv)
{
    int slot;

    if (argc != 2 || (slot = parse_slot (argv[1])) < 0) {
        warn ("Usage: on SLOT (0-15)\n");
        return;
    }
    read_power (slot);
}

static void cmd_measure (int argc, char **argv)
{
    int slot;

    if (argc != 2 || (slot = parse_slot (argv[1])) < 0) {
        warn ("Usage: measure SLOT (0-15)\n");
        return;
    }
    read_power_measure (slot);
}

static bool run_command (char *argz, size_t argz_len)
{
    int argc;
    char **argv;
    bool valid = true;

    argc = argz_count (argz, argz_len);
    argv = calloc (argc, sizeof (argv[0]));
    if (!argv)
        die ("out of memory\n");
    argz_extract (argz, argz_len, argv);
    assert (argc > 0); // already checked for empty/whitespace string

    if (!strcasecmp (argv[0], "help"))
        cmd_help ();
    else if (!strcasecmp (argv[0], "on"))
        cmd_on (argc, argv);
    else if (!strcasecmp (argv[0], "off"))
        cmd_off (argc, argv);
    else if (!strcasecmp (argv[0], "status"))
        cmd_status (argc, argv);
    else if (!strcasecmp (argv[0], "measure"))
        cmd_measure (argc, argv);
    else
        valid = false;
    free (argv);
    return valid;
}

int power_main (int argc, char *argv[])
{
    char buf[128] = { 0 };

    if (argc != 1 && argc != 2)
        die ("Usage: bramble powerman-helper [SRCADDR]\n");

    if (argc == 2) {
        char *endptr;

        errno = 0;
        srcaddr = strtol (argv[1], &endptr, 16);
        if (errno != 0 || *endptr != '\0' || srcaddr < 0 || srcaddr > 0x3f)
            die ("error parsing hexadecimal SRCADDR from command line\n");
    }
    else {
        int slot;

        if ((slot = slot_get ()) < 0) {
            warn ("WARNING: i2c: %s, assuming CAN address 02\n",
                  strerror (errno));
            srcaddr = 0x02;
        }
        else
            srcaddr = slot | CANMSG_ADDR_COMPUTE;
    }

    if ((canfd = can_open (BRAMBLE_CAN_INTERFACE, srcaddr)) < 0)
        die ("%s: %s\n", BRAMBLE_CAN_INTERFACE, strerror (errno));

    do {
        /* Drop trailing white space, including newline, if any
         */
        while (isspace (buf[strlen (buf) - 1]))
            buf[strlen (buf) - 1] = '\0';

        /* Parse command line into space-delimited arguments.
         */
        if (strlen (buf) > 0) {
            char *argz = NULL;
            size_t argz_len = 0;

            if (argz_create_sep (buf, ' ', &argz, &argz_len) != 0)
                die ("out of memory\n");
            if (!run_command (argz, argz_len))
                printf ("Type 'help' for a list of valid commands\n");
            free (argz);
        }

        printf ("power> ");
        fflush (stdout);

    } while (fgets (buf, sizeof (buf), stdin) != NULL);

    close (canfd);

    return 0;
}


/*
 * vi:ts=4 sw=4 expandtab
 */

