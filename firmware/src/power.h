/* SPDX-License-Identifier: GPL-3.0-or-later */

void power_init (void);

void power_set_state (bool enable);
bool power_get_state (void);

/* Measure current in mA
 * Calls taskYield().
 */
uint16_t power_measure_ma (void);

/* Measure voltage in mV
 * Calls taskYield().
 */
uint16_t power_measure_mv (void);

/*
 * vi:ts=4 sw=4 expandtab
 */
