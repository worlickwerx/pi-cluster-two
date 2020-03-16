/* SPDX-License-Identifier: GPL-3.0-or-later */

/* power.c - control power of raspberry pi
 *
 * power switch
 * - pulse PA6 to toggle (clocked on rising edge)
 *
 * power status
 * - read PA7 (0=off, 1=on)
 *
 * power measure
 * - read PB0 analog
 * - reading is proportional to current
 *
 * N.B. The state is unaffected by STM32 reset.
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
}

/*
 * vi:ts=4 sw=4 expandtab
 */
