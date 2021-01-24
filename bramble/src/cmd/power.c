/* SPDX-License-Identifier: GPL-3.0-or-later */

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

#include "src/libbramble/bramble.h"

static int my_slot;
static int canfd;

static void cmd_help (void)
{
    printf ("Commands: on, off, status, measure\n");
}

static int write_power (int slot, int val)
{
    struct canobj *obj;
    uint8_t data[1] = { val };

    if (!(obj = canobj_open_with (canfd, my_slot,
                                  slot, CANMSG_V1_OBJ_POWER))
             || canobj_write (obj, data, sizeof (data)) < 0)
        printf ("failed: %s\n", strerror (errno));
    else
        printf ("OK\n");
    canobj_close (obj);
}

static int read_power (int slot)
{
    struct canobj *obj;
    uint8_t data[8];
    int n;

    if (!(obj = canobj_open_with (canfd, my_slot,
                                  slot, CANMSG_V1_OBJ_POWER))
        || (n = canobj_read (obj, data, sizeof (data))) < 0)
        printf ("failed: %s\n", strerror (errno));
    else {
        printf ("%s\n", data[0] ? "on" : "off");
    }
    canobj_close (obj);
}


static int read_power_measure (int slot)
{
    struct canobj *obj;
    uint8_t data[8];
    int n;

    if (!(obj = canobj_open_with (canfd, my_slot,
                                  slot, CANMSG_V1_OBJ_POWER_MEASURE))
        || (n = canobj_read (obj, data, sizeof (data))) < 0)
        printf ("failed: %s\n", strerror (errno));
    else {
        uint16_t ma = be16toh (*(uint16_t *)&data[0]);
        printf ("%.3fA\n", 1E-3*ma);
    }
    canobj_close (obj);
}

static void cmd_on (int argc, char **argv)
{
    int slot;

    if (argc != 2 || (slot = slot_parse (argv[1])) < 0) {
        fprintf (stderr, "Usage: on SLOT (0-15)\n");
        return;
    }
    write_power (slot, 1);
}

static void cmd_off (int argc, char **argv)
{
    int slot;

    if (argc != 2 || (slot = slot_parse (argv[1])) < 0) {
        fprintf (stderr, "Usage: on SLOT (0-15)\n");
        return;
    }
    write_power (slot, 0);
}

static void cmd_status (int argc, char **argv)
{
    int slot;

    if (argc != 2 || (slot = slot_parse (argv[1])) < 0) {
        fprintf (stderr, "Usage: on SLOT (0-15)\n");
        return;
    }
    read_power (slot);
}

static void cmd_measure (int argc, char **argv)
{
    int slot;

    if (argc != 2 || (slot = slot_parse (argv[1])) < 0) {
        fprintf (stderr, "Usage: measure SLOT (0-15)\n");
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

    if (argc != 1)
        die ("Usage: bramble powerman-helper\n");

    if ((my_slot = slot_get ()) < 0)
        die ("error reading slot from i2c: %s\n", strerror (errno));

    if ((canfd = can_open (BRAMBLE_CAN_INTERFACE)) < 0)
        die ("error opening CAN interface\n");

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

