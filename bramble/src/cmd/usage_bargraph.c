/* SPDX-License-Identifier: GPL-3.0-or-later */

#if HAVE_CONFIG_H
# include "config.h"
#endif
#include <math.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include "src/libbramble/bramble.h"

/* Currently, if the actual number of cpus in /proc/stat does not match
 * NR_CPUS, we soldier on but complain about it, assuming it's a transient
 * I/O error.
 */
#define NR_CPUS 4

/* Display is a 5x7 matrix.
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

/* The values in val[] should range from 0 to 1.
 * Each will be represented as a bargraph with a width of one pixel.
 */
static void create_bargraphs (double *val,
                              int count,
                              uint8_t cols[NR_COLS])
{
    memset (cols, 0, sizeof (uint8_t) * NR_COLS);

    for (int i = 0; i < min (count, NR_COLS); i++) {
        int n = floor (val[i] * NR_ROWS); // n is the number of "on" pixels
        for (int j = NR_ROWS - 1; j >= 0; j--) { // top row is 0
            if (n-- == 0)
                break;
            cols[i] |= 1<<j;
        }
    }
}

/* Scale x with a range of [min:max] to a value with a range of [0:1].
 */
double scale (double min, double max, double x)
{
    double val = (x - min) / (max - min);
    if (val < 0)
        val = 0;
    if (val > 1)
        val = 1;
    return val;
}

void sighandler (int sig)
{
    int8_t cols[NR_COLS];

    memset (cols, 0, sizeof (cols));
    update_display (cols);
    _exit (0);
}

int usage_bargraph_main (int argc, char *argv[])
{
    struct proc_cpu last[NR_CPUS];
    struct proc_cpu cur[NR_CPUS];
    int count = 0;
    uint8_t cols[NR_COLS];
    int n;

    signal (SIGTERM, sighandler);

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
            double val[NR_COLS];
            int i;
            double temp_C;

            for (i = 0; i < min(NR_CPUS, NR_COLS - 1); i++)
                val[i] = proc_stat_calc_cpu_usage (&last[i], &cur[i]);
            temp_read (&temp_C);

            /* The last col is the temperature.
             * Scale a range of [30:85] to [0:1].
             * 30C = reasonable idle temp
             * 85C = pi clock throttling threshold
             */
            val[i++] = scale (30., 85., temp_C);

            create_bargraphs (val, i, cols);
            update_display (cols);
        }
        sleep (update_period);
    }

    return 0;
}

// vi:ts=4 sw=4 expandtab

