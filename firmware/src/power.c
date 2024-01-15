/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

/* power.c - control power of raspberry pi
 *
 * This was originally written for the pi 4, which exposes its PMIC:
 *
 * GLOBAL_EN (PB14) - enable
 *   Set to 0 to turn off the PMIC.
 *   Set to 1 to turn on the PMIC.
 *   This pin is pulled to 5V on the pi (PB14 is 5V tolerant on the STM32).
 *
 * RUN_PG (PA8) - power good
 *   If 0, power is off.
 *   If 1, power is on.
 *   This is a 3V3 signal.
 *
 * Pi 5 support was added later.  The pi 5 has a power button but does not
 * expose the PMIC:
 *
 * PI5_MODE (PA6) - pi 5 mode select
 *   Jumpered to ground for pi 5 mode.
 *   Floating (internal pull to 3V3) for the previous behavior
 *
 * PI5_BUTTON (PB14) - power button (was GLOBAL_EN)
 *   Set to 0 to depress the button.
 *   Set to 1 to release the button
 *
 * PI_3V3 (PA8) - pulled to pi 3V3 power bus (was RUN_PG)
 *   If 0, power is off.
 *   If 1, power is on.
 *
 * Soft shutdown support is available via the 'gpio-shutdown' device tree
 * overlay.  Add the following to /boot/config.txt:
 *
 *   dtoverlay=gpio-shutdown,gpio_pin=27,active_low=1,gpio_pull=up
 *   gpio=26=op,dh
 *
 * GPIO27 (PA4)
 *   Pulse low and then high to trigger shutdown.

 * GPIO26 (PA5)
 *   Wait for transition high to low to indicate shutdown complete.
 */

#include <string.h>

#include "librtos/FreeRTOS.h"
#include "librtos/task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/f1/bkp.h>
#include <libopencm3/stm32/f1/pwr.h>

#include "trace.h"
#include "power.h"
#include "matrix.h"

static TaskHandle_t shutdown_task_handle = NULL;

static bool pi5_mode (void)
{
    if (gpio_get (GPIOA, GPIO6))
        return false;
    return true;
}

static bool pi_run_pg_get (void)
{
    if (gpio_get (GPIOA, GPIO8))
        return true;
    return false;
}

static void pi_global_en_set (bool val)
{
    if (val)
        gpio_set (GPIOB, GPIO14);
    else
        gpio_clear (GPIOB, GPIO14);
}

/* Read from port configured as output should return the last value written.
 */
static bool pi_global_en_get (void)
{
    if (gpio_get (GPIOB, GPIO14))
        return true;
    return false;
}

static uint16_t backup_get_dr1 (void)
{
    return BKP_DR1;
}

static void backup_set_dr1 (uint16_t val)
{
    PWR_CR |= PWR_CR_DBP;    // write access enable
    BKP_DR1 = val;
    PWR_CR &= ~PWR_CR_DBP;   // write access disable
}

bool power_get_state (void)
{
    return pi_run_pg_get ();
}

void power_set_state (bool val)
{
    if (pi5_mode ()) {
        if (pi_run_pg_get () == val)
            return;
        /* Short press to turn on the pi 5 from off.
         */
        if (val) {
            pi_global_en_set (0);
            vTaskDelay (pdMS_TO_TICKS (100));
            pi_global_en_set (1);
            while (!pi_run_pg_get ())
                vTaskDelay (pdMS_TO_TICKS (1));
        }
        /* Long press to turn off the pi 5 from on.
         */
        else {
            pi_global_en_set (0);
            vTaskDelay (pdMS_TO_TICKS (1));
            while (pi_run_pg_get ())
                vTaskDelay (pdMS_TO_TICKS (1));
            pi_global_en_set (1);
        }
    }
    else {
        /* If GLOBAL_EN is high and RUN_PG is low, the pi may have turned off
         * its own PMIC (e.g. shutdown -h).  In that case, GLOBAL_EN needs to be
         * pulsed low for >= 1ms.
         */
        if (val && pi_global_en_get () && !pi_run_pg_get ()) {
            pi_global_en_set (false);
            vTaskDelay (pdMS_TO_TICKS (1));
        }

        /* Set GLOBAL_EN to desired state, then wait for RUN_PG to change before
         * returning.
         */
        pi_global_en_set (val);
        while (pi_run_pg_get () != val)
            vTaskDelay (pdMS_TO_TICKS (1));
    }
    backup_set_dr1 (val ? 1 : 0);
}

static void power_task (void *args __attribute((unused)))
{
    const int loop_msec = 10;
    const int button_short_msec = 30; // debounce limit
    const int button_long_msec = 2000;
    bool button_press = false;
    int button_msec = 0;

    for (;;) {
        /* Ensure that the red LED tracks RUN_PG.
         */
        matrix_set_red (pi_run_pg_get () ? 1 : 0);

        /* Perform OS friendly shutdown on short press of power button,
         * or hard power off on long press.  Or if the power is off, turn it
         * on with a short press.
         */
        if (!gpio_get (GPIOA, GPIO3)) { // button depressed
            if (button_press)
                button_msec += loop_msec;
            else {
                button_press = true;
                button_msec = 0;
            }
            if (button_msec >= button_long_msec) {
                power_set_state (false);
            }
            else if (button_msec >= button_short_msec) {
                if (power_get_state ()) {
                    power_shutdown ();
                }
                else {
                    power_set_state (true);
                }
            }
        }
        else { // button released
            button_press = false;
        }

        vTaskDelay (pdMS_TO_TICKS (loop_msec));
    }
}

/* Pulse the PI_HALT line low momentarily when notified by power_shutdown ().
 */
static void shutdown_task (void *args __attribute((unused)))
{
    for (;;) {
        ulTaskNotifyTake (pdTRUE, portMAX_DELAY);

        gpio_clear (GPIOA, GPIO4);
        vTaskDelay (pdMS_TO_TICKS (1000));
        gpio_set (GPIOA, GPIO4);
    }
}

void power_shutdown (void)
{
    xTaskNotifyGive (shutdown_task_handle);
}

bool power_is_shutdown (void)
{
    return gpio_get (GPIOA, GPIO5) ? false : true;
}

void power_init (bool por_flag)
{
    rcc_periph_clock_enable (RCC_GPIOA);
    rcc_periph_clock_enable (RCC_GPIOB);

    /* Enable power and backup interface clocks
     */
    rcc_peripheral_enable_clock (&RCC_APB1ENR, RCC_APB1ENR_PWREN);
    rcc_peripheral_enable_clock (&RCC_APB1ENR, RCC_APB1ENR_BKPEN);

    /* Configure inputs
     */
    gpio_set_mode (GPIOA,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_FLOAT,
                   GPIO8); // RUN_PG (pulled to +3V3 with 10K on the pi)
    gpio_set_mode (GPIOA,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_FLOAT,
                   GPIO5); // RUNNING
    gpio_set_mode (GPIOA,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_PULL_UPDOWN,
                   GPIO3); // BUTTON
    gpio_set (GPIOA, GPIO3); // pull up
    gpio_set_mode (GPIOA,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_PULL_UPDOWN,
                   GPIO6); // PI5 select
    gpio_set (GPIOA, GPIO6); // pull up

    /* Configure outputs
     * N.B. the GLOBAL_EN output will revert to an input during reset,
     * possibly allowing the pi PMIC to turn on briefly until we resolve
     * it below.
     */
    gpio_set (GPIOA, GPIO4);
    gpio_set_mode (GPIOA,
                   GPIO_MODE_OUTPUT_2_MHZ,
                   GPIO_CNF_OUTPUT_OPENDRAIN,
                   GPIO4); // SHUTDOWN (active low)

    gpio_set (GPIOB, GPIO14);
    gpio_set_mode (GPIOB,
                   GPIO_MODE_OUTPUT_2_MHZ,
                   GPIO_CNF_OUTPUT_OPENDRAIN,
                   GPIO14); // GLOBAL_EN (pulled to +5V with 100K on the pi)

    if (pi5_mode ()) {
        /* Initialize to "button unpressed".
         * The pi 5 always starts when power is initially applied.
         */
        gpio_set (GPIOB, GPIO14); // initialize to "button unpressed"
    }
    else {
        /* Lower GLOBAL_EN if this is a power on reset (vs warm reset)
         * OR if previous state from backup register is off.
         * Backup register preserves last power state across a reset
         * (only a warm reset if battery is not attached).
         */
        if (por_flag || backup_get_dr1 () == 0)
            gpio_clear (GPIOB, GPIO14);

        if (por_flag)
            backup_set_dr1 (0);
    }

    xTaskCreate (power_task,
                "power",
                 100,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 NULL);

    xTaskCreate (shutdown_task,
                 "shutdown",
                 200,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 &shutdown_task_handle);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
