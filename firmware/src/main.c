#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/nvic.h>

#include "blink.h"
#include "address.h"
#include "matrix.h"
#include "power.h"
#include "trace.h"

extern void vApplicationStackOverflowHook (xTaskHandle *pxTask,
                                           signed portCHAR *pcTaskName);


void vApplicationStackOverflowHook (xTaskHandle *task __attribute((unused)),
                                    signed portCHAR *name __attribute((unused)))
{
    for (;;)
       ;
}

int main (void)
{
    rcc_clock_setup_in_hse_8mhz_out_72mhz ();    // Use this for "blue pill"

    blink_init ();
    matrix_init ();
    address_init ();
    power_init ();

    trace_puts ("Hello world!\r\n");

    vTaskStartScheduler ();
    /*NOTREACHED*/
    for (;;)
        ;
    return 0;
}

/*
 * vi:ts=4 sw=4 expandtab
 */
