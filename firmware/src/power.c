/* SPDX-License-Identifier: GPL-3.0-or-later */

/* power.c - control power of raspberry pi
 *
 * power switch
 * - pulse PA6 to toggle (clocked on rising edge)
 *
 * power status
 * - read PA7 (0=off, 1=on)
 *
 * power measure (I)
 * - read PB0 analog
 *
 * power measure (V)
 * - read PB1 analog
 *
 * N.B. The state is unaffected by STM32 reset.
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
    bool state;

    taskENTER_CRITICAL ();
    state = power_get_state ();
    if (state != enable)
        pulse_pa6 ();
    state = power_get_state ();
    taskEXIT_CRITICAL ();

    matrix_set_red (state ? 1 : 0);
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

static void measure_task (void *args __attribute((unused)))
{
    bool state = power_get_state ();

    matrix_set_red (state ? 1 : 0);

    for (;;) {
        uint16_t ma = power_measure_ma ();
        uint16_t mv = power_measure_mv ();

        taskENTER_CRITICAL ();
        last_measure_ma = ma;
        last_measure_mv = mv;
        taskEXIT_CRITICAL ();

        vTaskDelay (pdMS_TO_TICKS (500));
    }
}


void power_init (void)
{
    rcc_periph_clock_enable (RCC_GPIOA);
    rcc_periph_clock_enable (RCC_GPIOB);

    gpio_set_mode (GPIOA,
                   GPIO_MODE_OUTPUT_2_MHZ,
                   GPIO_CNF_OUTPUT_PUSHPULL,
                   GPIO6); // PWR_TOGGLE

    gpio_set_mode (GPIOA,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_PULL_UPDOWN,
                   GPIO7); // PWR_SENSE

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

    xTaskCreate (measure_task,
                "power",
                 100,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 NULL);
}


/*
 * vi:ts=4 sw=4 expandtab
 */
