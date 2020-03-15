/* SPDX-License-Identifier: GPL-3.0-or-later */

/* address.c - read molliebus geographic slot address
 */

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "address.h"

uint8_t address_get (void)
{
    uint8_t val = 0;

    if (gpio_get (GPIOC, GPIO15))
        val |= 1;
    if (gpio_get (GPIOC, GPIO14))
        val |= 2;
    if (gpio_get (GPIOB, GPIO7))
        val |= 4;
    if (gpio_get (GPIOB, GPIO6))
        val |= 8;
    if (gpio_get (GPIOB, GPIO5))
        val |= 16;
    return val;
}

/* Configure 5 GA bits as inputs with pullups.
 */
void address_init (void)
{
    rcc_periph_clock_enable (RCC_GPIOB);
    rcc_periph_clock_enable (RCC_GPIOC);

    gpio_set_mode (GPIOC,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_PULL_UPDOWN,
                   GPIO15 | GPIO14); // GA0, GA1
    gpio_set (GPIOC, GPIO15 | GPIO14);

    gpio_set_mode (GPIOB,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_PULL_UPDOWN,
                   GPIO7 | GPIO6 | GPIO5); // GA2, GA3, GA4
    gpio_set (GPIOB, GPIO7 | GPIO6);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
