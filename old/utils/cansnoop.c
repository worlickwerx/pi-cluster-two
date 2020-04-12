/* cansnoop - display decoded CAN bus traffic */

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "canmgr_proto.h"
#include "canmgr_dump.h"
#include "lxcan.h"
#include "monotime.h"

static double start;

void dump_frame (struct canmgr_frame *fr)
{ 
    char dump[80]; 
    canmgr_dump (fr, dump, sizeof (dump));
    printf ("%07.3f %s\n", monotime_since (start), dump);
}

int main (int argc, char *argv[])
{
    struct canmgr_frame in;
    int s;

    if (argc != 1) {
        fprintf (stderr, "Usage: cansnoop\n");
        exit (1);
    }
    start = monotime ();

    if ((s = lxcan_open ("can0")) < 0) {
        fprintf (stderr, "lxcan_open: %m\n");
        exit (1);
    }
    for (;;) {
        if (lxcan_recv (s, &in) < 0) {
            fprintf (stderr, "lxcan_recv: %m\n");
            continue;
        }
        dump_frame (&in);
    }
    lxcan_close (s);

    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
