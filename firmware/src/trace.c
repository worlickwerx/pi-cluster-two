/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

/* Print to SWO trace output.
 * Access with "stlink-trace -c 72".
 * If ITM port is not enabled (no debugger is attached), do nothing.
 */

#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/cm3/itm.h>

#include "trace.h"
#include "miniprintf.h"

static bool trace_enabled (void)
{
    if ((ITM_TCR & ITM_TCR_ITMENA) && ITM_TER[0] & 1)
        return true;
    return false;
}

static void trace_putc (char c)
{
    while (!(ITM_STIM8(0) & ITM_STIM_FIFOREADY))
        ;
    ITM_STIM8(0) = c;
}

void trace_printf (const char *fmt, ...)
{
    if (trace_enabled ()) {
        va_list args;
        va_start (args, fmt);
        (void)mini_vprintf_cooked (trace_putc, fmt, args);
        va_end (args);
    }
}

void trace_fatal (const char *msg, const char *file, int line)
{
    trace_printf ("fatal error (%s::%d): %s\n", file, line, msg);
    while (1)
        ;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
