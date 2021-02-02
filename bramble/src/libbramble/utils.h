/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _BRAMBLE_UTILS_H
#define _BRAMBLE_UTILS_H

#include <stdarg.h>

void die (const char *fmt, ...);
void warn (const char *fmt, ...);

double monotime (void);
double monotime_since (double t);

int slot_parse (const char *s);
int slot_get (void);

#endif /* !_BRAMBLE_UTILS */

/*
 * vi:ts=4 sw=4 expandtab
 */
