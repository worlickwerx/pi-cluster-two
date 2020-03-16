/* SPDX-License-Identifier: GPL-3.0-or-later */

/* power.c - control power of raspberry pi
 *
 * Power switch state is toggled by rising clock edge of PA6.
 * The power switch state may be read on PA7.
 * The current may be measured via an INA169 on PB0 (analog).
 *
 * N.B. PA6 has an external pull-down resistor to ensure power state does
 * not change when the STM32 is reset.  On power-up, the power state will
 * be pre-set to off.
 */

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "power.h"

static void pulse_pa6 (void)
{
    gpio_set (GPIOA, GPIO6);
    gpio_clear (GPIOA, GPIO6);
}

bool power_get_state (void)
{
    if (gpio_get (GPIOA, GPIO7))
        return true;
    return false;
}

void power_set_state (bool enable)
{
    if (power_get_state () != enable)
        pulse_pa6 ();
}

void power_init (void)
{
    rcc_periph_clock_enable (RCC_GPIOA);

    gpio_set_mode (GPIOA,
                   GPIO_MODE_OUTPUT_2_MHZ,
                   GPIO_CNF_OUTPUT_PUSHPULL,
                   GPIO6); // PWR_TOGGLE

    gpio_set_mode (GPIOA,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_PULL_UPDOWN,
                   GPIO7); // PWR_SENSE
    //gpio_set (GPIOA, GPIO7); // weak pull-up
}

/*
 * vi:ts=4 sw=4 expandtab
 */
