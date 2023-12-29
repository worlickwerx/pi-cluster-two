/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#ifndef _FIRMWARE_POWER_H
#define _FIRMWARE_POWER_H

void power_init (bool por_flag);

void power_set_state (bool enable); // can sleep
bool power_get_state (void);

void power_shutdown (void);
bool power_is_shutdown (void);

#endif /* !_FIRMWARE_POWER_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
