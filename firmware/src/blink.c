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
        gpio_clear (GPIOC, GPIO13); // on
        vTaskDelay (pdMS_TO_TICKS (1));
        gpio_set (GPIOC, GPIO13); // off
        vTaskDelay (pdMS_TO_TICKS (2000));
    }
}

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook (xTaskHandle *task __attribute((unused)),
                                    signed portCHAR *name __attribute((unused)))
{
    static int count;
    for (;;) {
        gpio_toggle (GPIOC, GPIO13);
        for (count = 0; count < 1000000; count++)
            ;
    }
}
#endif

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
