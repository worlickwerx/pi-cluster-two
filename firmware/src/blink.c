/* SPDX-License-Identifier: GPL-3.0-or-later */

/* blink.c - blink PC13 LED to let the world know RTOS is running
 */

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "blink.h"

static void blink_task (void *args __attribute((unused)))
{
    for (;;) {
        gpio_toggle (GPIOC, GPIO13);
        vTaskDelay (pdMS_TO_TICKS (500));
    }
}

void blink_init (void)
{
    rcc_periph_clock_enable (RCC_GPIOC);
    gpio_set_mode (GPIOC,
                   GPIO_MODE_OUTPUT_2_MHZ,
                   GPIO_CNF_OUTPUT_PUSHPULL,
                   GPIO13);

    xTaskCreate (blink_task,
                 "LED",
                 100,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 NULL);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
