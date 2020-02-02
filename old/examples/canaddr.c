/* canaddr - display module,node address of this node */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "address.h"

int main (int argc, char *argv[])
{
    uint8_t module, node;

    if (argc != 1) {
        fprintf (stderr, "Usage: canaddr\n");
        exit (1);
    }
    if (can_address_get (&module, &node) < 0) {
        fprintf (stderr, "error reading GPIO: %m\n");
        exit (1);
    }
    printf ("%d,%d\n", module, node);
    exit (0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
