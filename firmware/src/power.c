/* SPDX-License-Identifier: GPL-3.0-or-later */

/* power.c - control power of raspberry pi
 *
 * power switch
 * - pulse PA6 to clock flip flop (clocked on rising edge)
 * - write PB14 (0=off, 1=on), connected to pi GLOBAL_EN
 *
 * power status
 * - read PA7 (0=off, 1=on), connected to flip flop Q output
 * - read PA8 (0=off, 1=on), connecting to pi RUN_PG
 *
 * The flip flop powers up in a known (off) state, then retains its
 * state across a STM32 reset.  On startup, set GLOBAL_EN to match flip flop Q,
 * and toggle flip flop on each power state change.
 *
 * N.B. RUN_PG is 3V3, but GLOBAL_EN is pulled externally to 5V.
 * PB14 is a 5V tolerant pin.
 */

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>

#include "trace.h"
#include "power.h"
#include "matrix.h"

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

static void flipflop_toggle (void)
{
    gpio_set (GPIOA, GPIO6);
    gpio_clear (GPIOA, GPIO6);
}

static bool flipflop_get (void)
{
    if (gpio_get (GPIOA, GPIO7))
        return true;
    return false;
}

bool power_get_state (void)
{
    return pi_run_pg_get ();
}

void power_set_state (bool val)
{
    /* If GLOBAL_EN is high and RUN_PG is low, the pi may have turned off
     * its own PMIC (e.g. shutdown -h).  In that case, GLOBAL_EN needs to be
     * pulsed low for >= 1ms.
     */
    if (val && pi_global_en_get () && !pi_run_pg_get ()) {
        pi_global_en_set (false);
        vTaskDelay (pdMS_TO_TICKS (1));
    }

    /* Set GLOAL_EN to desired state, then wait for RUN_PG to change before
     * returning.
     */
    pi_global_en_set (val);
    while (pi_run_pg_get () != val)
        vTaskDelay (pdMS_TO_TICKS (1));
}

static void power_task (void *args __attribute((unused)))
{
    pi_global_en_set (flipflop_get ());

    for (;;) {
        bool run_pg = pi_run_pg_get ();
        if (run_pg != flipflop_get ())
            flipflop_toggle ();

        matrix_set_red (run_pg ? 1 : 0);

        vTaskDelay (pdMS_TO_TICKS (100));

    }
}

void power_init (void)
{
    rcc_periph_clock_enable (RCC_GPIOA);
    rcc_periph_clock_enable (RCC_GPIOB);

    gpio_set (GPIOB, GPIO14); // careful not to take GLOBAL_EN low during init

    gpio_set_mode (GPIOB,
                   GPIO_MODE_OUTPUT_2_MHZ,
                   GPIO_CNF_OUTPUT_OPENDRAIN,
                   GPIO14); // GLOBAL_EN

    gpio_set_mode (GPIOA,
                   GPIO_MODE_OUTPUT_2_MHZ,
                   GPIO_CNF_OUTPUT_PUSHPULL,
                   GPIO6); // PWR_TOGGLE

    gpio_set_mode (GPIOA,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_PULL_UPDOWN,
                   GPIO7 | GPIO8); // PWR_SENSE, RUN_PG

    xTaskCreate (power_task,
                "power",
                 100,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 NULL);
}


/*
 * vi:ts=4 sw=4 expandtab
 */
