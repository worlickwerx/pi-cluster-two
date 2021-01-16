/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef _FIRMWARE_POWER_H
#define _FIRMWARE_POWER_H

void power_init (void);

void power_set_state (bool enable);
bool power_get_state (void);

void power_get_measurements (uint16_t *ma, uint16_t *mv);

#endif /* !_FIRMWARE_POWER_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
