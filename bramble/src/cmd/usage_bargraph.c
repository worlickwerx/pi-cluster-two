/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "src/libbramble/bramble.h"

/* Currently, if the actual number of cpus in /proc/stat does not match
 * NR_CPUS, we soldier on but complain about it, assuming it's a transient
 * I/O error.
 */
#define NR_CPUS 4

/* The display can show a bargraph of up to 5 cpus with 7 bars each.
 * This could be transposed to show up to 7 cpus with 5 bars each, if needed.
 * After that, maybe we need a different display, or just show one bar for
 * the overall usage.
 */
#define NR_COLS 5
#define NR_ROWS 7

#ifndef min
#define min(x,y) ((x)>(y)?(y):(x))
#endif

/* How often (seconds) is the display updated?
 */
static const int update_period = 1;

static void update_display (uint8_t cols[NR_COLS])
{
    int fd;

    if ((fd = i2c_open (BRAMBLE_I2C_DEVICE, I2C_ADDRESS)) < 0) {
        warn ("%s: %s\n", BRAMBLE_I2C_DEVICE, strerror (errno));
        return;
    }
    if (i2c_write (fd, I2C_REG_MATRIX_RAW, cols, NR_COLS) < 0)
        warn ("i2c write: %s\n", strerror (errno));
    close (fd);
}

static void create_bargraphs (double used[NR_CPUS], uint8_t cols[NR_COLS])
{
    for (int i = 0; i < min (NR_CPUS, NR_COLS); i++) {
        int n = floor (used[i] * NR_ROWS); // n is the number of "on" pixels
        for (int j = NR_ROWS - 1; j >= 0; j--) { // top row is 0
            if (n-- == 0)
                break;
            cols[i] |= 1<<j;
        }
    }
}

int usage_bargraph_main (int argc, char *argv[])
{
    struct proc_cpu last[NR_CPUS];
    struct proc_cpu cur[NR_CPUS];
    int count = 0;
    uint8_t cols[NR_COLS];
    int n;

    for (;;) {
        memcpy (&last, &cur, sizeof (cur));
        memset (&cur, 0, sizeof (cur));
        n = proc_stat_get_cpus (cur, NR_CPUS);
        if (n != NR_CPUS) {
            /* N.B. If proc_stat_get_cpus() fails for some reason, it could
             * return -1 or any number between 0 and the actual number of CPUs.
             * Unfilled structs are zeroed above, and proc_stat_calc_cpu_usage()
             * handles a zeroed sample by returning zero usage.
             */
            warn ("read %d cpus from /proc/stats, expected %d", n, NR_CPUS);
        }
        if (count++ > 0) {
            double used[NR_CPUS];

            for (int i = 0; i < NR_CPUS; i++)
                used[i] = proc_stat_calc_cpu_usage (&last[i], &cur[i]);

            memset (cols, 0, sizeof (cols));
            create_bargraphs (used, cols);
            update_display (cols);
        }
        sleep (update_period);
        count++;
    }

    return 0;
}

// vi:ts=4 sw=4 expandtab

