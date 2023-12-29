/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

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

    if (gpio_get (GPIOA, GPIO0))
        val |= 1;
    if (gpio_get (GPIOA, GPIO1))
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
    rcc_periph_clock_enable (RCC_GPIOA);

    gpio_set_mode (GPIOA,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_PULL_UPDOWN,
                   GPIO0 | GPIO1); // GA0, GA1
    gpio_set (GPIOA, GPIO0 | GPIO1);

    gpio_set_mode (GPIOB,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_PULL_UPDOWN,
                   GPIO7 | GPIO6 | GPIO5); // GA2, GA3, GA4
    gpio_set (GPIOB, GPIO7 | GPIO6);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
