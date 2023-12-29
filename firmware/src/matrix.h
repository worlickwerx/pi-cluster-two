/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#ifndef _FIRMWARE_MATRIX_H
#define _FIRMWARE_MATRIX_H

void matrix_init (void);

/* Write ascii character to 5x7 matrix.
 * It will be translated to an image using font5x7.h.
 */
void matrix_set_char (char c);
void matrix_set_char_from_isr (char c);

/* Write columns to 5x7 matrix.
 * (array of 5 column bytes containing 7 row bits each).
 */
void matrix_set (const uint8_t cols[]);
void matrix_set_from_isr (const uint8_t cols[]);

/* Pulse the green LED for a short duration.
 */
void matrix_pulse_green (void);

/* Change the red LED state.
 */
void matrix_set_red (uint8_t val);

#endif /* _FIRMWARE_MATRIX_H */

/*
 * vi:ts=4 sw=4 expandtab
 */
