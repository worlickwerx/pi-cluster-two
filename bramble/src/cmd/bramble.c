/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <stdio.h>
#include <string.h>

#include "src/libbramble/bramble.h"

int cansnoop_main (int argc, char **argv);
int canping_main (int argc, char **argv);
int slot_main (int argc, char **argv);
int version_main (int argc, char **argv);
int power_main (int argc, char **argv);
int console_main (int argc, char **argv);
int led_main (int argc, char **argv);
int usage_bargraph_main (int argc, char **argv);

struct subcmd {
    const char *name;
    const char *desc;
    int (*main)(int argc, char **argv);
};

static const struct subcmd builtins[] = {
    { "cansnoop",           "snoop CAN traffic",  cansnoop_main },
    { "canping",            "send echo request to CAN hosts",  canping_main },
    { "slot",               "print backplane slot number",  slot_main },
    { "version",            "print software version",  version_main },
    { "powerman-helper",    "power on/off slots",  power_main},
    { "conman-helper",      "netcat-like CAN console access",  console_main},
    { "led",                "update LED matrix display",  led_main},
    { "usage-bargraph",
      "show cpu utilization on LED matrix",
       usage_bargraph_main},
};
static const int builtins_count = sizeof (builtins) / sizeof (builtins[0]);

int usage (void)
{
    int i;

    warn ("Usage: bramble CMD [arg ...]\n"
             "where CMD is:\n");
    for (i = 0; i < builtins_count; i++)
        warn ("  %-20s%s\n", builtins[i].name, builtins[i].desc);
    return 1;
}

int main (int argc, char *argv[])
{
    if (argc > 1) {
        int i;

        for (i = 0; i < builtins_count; i++) {
            if (!strcmp (argv[1], builtins[i].name))
                return builtins[i].main (argc - 1, argv + 1);
        }
    }
    return usage ();
}

/*
 * vi:ts=4 sw=4 expandtab
 */

