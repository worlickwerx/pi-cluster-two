/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _FIRMWARE_POWER_H
#define _FIRMWARE_POWER_H

void power_init (void);

void power_set_state (bool enable); // can sleep
bool power_get_state (void);

#endif /* !_FIRMWARE_POWER_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
