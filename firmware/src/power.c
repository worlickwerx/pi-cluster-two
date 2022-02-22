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
 * power measure (I)
 * - read PB0 analog
 *
 * power measure (V)
 * - read PB1 analog
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

static uint16_t last_measure_mv;
static uint16_t last_measure_ma;

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

void power_get_measurements (uint16_t *ma, uint16_t *mv)
{
    taskENTER_CRITICAL ();
    if (ma)
        *ma = last_measure_ma;
    if (mv)
        *mv = last_measure_mv;
    taskEXIT_CRITICAL ();
}

static uint16_t read_adc (uint8_t channel)
{
    adc_set_sample_time (ADC1, channel, ADC_SMPR_SMP_239DOT5CYC);
    adc_set_regular_sequence (ADC1, 1, &channel);
    adc_start_conversion_direct (ADC1);
    while (!adc_eoc (ADC1))
        taskYIELD();
    return adc_read_regular (ADC1);
}

/* Measure 5V output through 50/50 voltage divider.
 * With 3V3 reference and 12 bit range, 5.0V should read as 3103.
 * Can sleep.
 */
static uint16_t power_measure_mv (void)
{
    uint16_t val = read_adc (ADC_CHANNEL9);
    return (val * 1000) / 620;
}

/* Assumes INA169 R_l=10K (gain 10X), R_shunt=0.1 ohm, V_ref=3.3V.
 * ADC range is 12 bits = 4096 values.
 * N.B. R_shunt drops 5V to 4.8V at 2A
 * Can sleep.
 */
static uint16_t power_measure_ma (void)
{
    uint16_t val = read_adc (ADC_CHANNEL8);
    return (val * 1000) / 1230;
}

static void power_task (void *args __attribute((unused)))
{
    pi_global_en_set (flipflop_get ());

    for (;;) {
        uint16_t ma = power_measure_ma ();
        uint16_t mv = power_measure_mv ();

        taskENTER_CRITICAL ();
        last_measure_ma = ma;
        last_measure_mv = mv;
        taskEXIT_CRITICAL ();

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

    gpio_set_mode (GPIOB,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_ANALOG,
                   GPIO0 | GPIO1 ); // PWR_MEASURE_I, PWR_MEASURE_V

    rcc_peripheral_enable_clock (&RCC_APB2ENR, RCC_APB2ENR_ADC1EN);
    adc_power_off (ADC1);
    rcc_peripheral_reset (&RCC_APB2RSTR, RCC_APB2RSTR_ADC1RST);
    rcc_peripheral_clear_reset (&RCC_APB2RSTR, RCC_APB2RSTR_ADC1RST);
    rcc_set_adcpre (RCC_CFGR_ADCPRE_PCLK2_DIV6); // Set. 12MHz, Max. 14MHz
    adc_set_dual_mode (ADC_CR1_DUALMOD_IND);     // Independent mode
    adc_disable_scan_mode (ADC1);
    adc_set_right_aligned (ADC1);
    adc_set_single_conversion_mode (ADC1);
    adc_set_sample_time (ADC1, ADC_CHANNEL8, ADC_SMPR_SMP_239DOT5CYC);
    adc_set_sample_time (ADC1, ADC_CHANNEL9, ADC_SMPR_SMP_239DOT5CYC);
    adc_enable_temperature_sensor ();
    adc_power_on (ADC1);
    adc_reset_calibration (ADC1);
    adc_calibrate_async (ADC1);
    while (adc_is_calibrating (ADC1))
        ;

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
