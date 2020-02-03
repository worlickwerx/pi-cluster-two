#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>


extern void vApplicationStackOverflowHook (xTaskHandle *pxTask,
                                           signed portCHAR *pcTaskName);


void vApplicationStackOverflowHook (xTaskHandle *task __attribute((unused)),
                                    signed portCHAR *name __attribute((unused)))
{
    for (;;)
       ;
}

static void task1 (void *args __attribute((unused)))
{
    for (;;) {
        gpio_toggle (GPIOC, GPIO13);
        vTaskDelay (pdMS_TO_TICKS (500));
    }
}

int main (void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz ();    // Use this for "blue pill"
    rcc_periph_clock_enable (RCC_GPIOC);

    gpio_set_mode (GPIOC,
                   GPIO_MODE_OUTPUT_2_MHZ,
                   GPIO_CNF_OUTPUT_PUSHPULL,
                   GPIO13);

    xTaskCreate (task1,
                 "LED",
                 100,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 NULL);

    vTaskStartScheduler ();
    /*NOTREACHED*/
    for (;;)
        ;
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
