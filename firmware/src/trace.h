/* SPDX-License-Identifier: GPL-3.0-or-later */

void trace_printf (const char *format,...)
		   __attribute((format(printf,1,2)));

void trace_fatal (const char *msg, const char *file, int line);

#define FATAL(msg)   trace_fatal (msg, __FILE__, __LINE__)
