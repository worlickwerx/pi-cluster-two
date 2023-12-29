/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#ifndef _FIRMWARE_TRACE_H
#define _FIRMWARE_TRACE_H

void trace_printf (const char *format,...)
		   __attribute((format(printf,1,2)));

void trace_fatal (const char *msg, const char *file, int line);

#define FATAL(msg)   trace_fatal (msg, __FILE__, __LINE__)

#endif /* !_FIRMWARE_TRACE_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
