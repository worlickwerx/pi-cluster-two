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
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "src/libbramble/bramble.h"

#define PATH_PROC_STAT "/proc/stat"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

static bool read_proc_stat_cpu (FILE *f, struct proc_cpu *cpu)
{
    char buf[2048];
    struct proc_cpu tmp;
    char *cp;

    if (fgets (buf, sizeof (buf), f) == NULL
        || !(cp = strchr (buf, ' '))
        || (cp - buf) >= CPU_NAME_SIZE // cpu.name would overflow
        || sscanf (buf,
                   "%[^ ] "
                   "%d %d %d %d %d %d %d %d %d %d",
                   tmp.name,
                   &tmp.user,
                   &tmp.nice,
                   &tmp.system,
                   &tmp.idle,
                   &tmp.iowait,
                   &tmp.irq,
                   &tmp.softirq,
                   &tmp.steal,
                   &tmp.guest,
                   &tmp.guest_nice) != 11
        || strncmp (tmp.name, "cpu", 3) != 0)
        return false;
    *cpu = tmp;
    return true;
}

/* read the first entry which should be called "cpu" (no numerical suffix).
 */
int proc_stat_get_cpu (struct proc_cpu *cpu)
{
    FILE *f;
    struct proc_cpu tmp;

    if (!(f = fopen (PATH_PROC_STAT, "r")))
        return -1;
    if (!read_proc_stat_cpu (f, &tmp)
        || strcmp (tmp.name, "cpu") != 0) {
        fclose (f);
        return -1;
    }
    fclose (f);
    *cpu = tmp;
    return 0;
}

/* skip the first entry called "cpU", then read the next 'count' entries.
 */
int proc_stat_get_cpus (struct proc_cpu *cpu, size_t count)
{
    FILE *f;
    struct proc_cpu tmp;
    int n = 0;

    if (!(f = fopen (PATH_PROC_STAT, "r")))
        return -1;
    if (!read_proc_stat_cpu (f, &tmp)
        || strcmp (tmp.name, "cpu") != 0)
        return -1;
    while (n < count) {
        if (!read_proc_stat_cpu (f, &cpu[n]))
            break;
        n++;
    }
    fclose (f);
    return n;
}

static int cpu_idle (struct proc_cpu *x)
{
    return x->idle + x->iowait;
}

static int cpu_total (struct proc_cpu *x)
{
    return cpu_idle (x)
         + x->user
         + x->nice
         + x->system
         + x->irq
         + x->softirq
         + x->steal;
}

double proc_stat_calc_cpu_usage (struct proc_cpu *sample1,
                                 struct proc_cpu *sample2)
{
    double total[2] = { cpu_total (sample1), cpu_total (sample2) };
    double idle[2] =  { cpu_idle (sample1),  cpu_idle (sample2) };
    double total_delta = total[1] - total[0];
    double idle_delta = idle[1] - idle[0];

    if (total[0] == 0
        || total[1] == 0
        || total_delta <= 0)
        return 0; // bad sample?

    return (total_delta - idle_delta) / total_delta;
}

/*
 * vi:ts=4 sw=4 expandtab
 */

