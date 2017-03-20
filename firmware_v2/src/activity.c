#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "activity.h"
#include "util.h"

const uint16_t activity_pin = GPIO_PIN_5;
GPIO_TypeDef *activity_port = GPIOB;

const uint8_t activity_period = 10; // millsec
uint32_t activity_t0;
uint8_t activity_state = 0;

static void activity_write (uint8_t val)
{
    HAL_GPIO_WritePin (activity_port, activity_pin,
                       val ? GPIO_PIN_SET : GPIO_PIN_RESET);
    activity_state = val;
}

void activity_setup (void)
{
    GPIO_InitTypeDef g;

    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = activity_pin;
    HAL_GPIO_Init (activity_port, &g);

    /* visual self-test */
    activity_write (1);
    HAL_Delay (200);
    activity_write (0);

    itm_printf ("Activity: initialized\n");
}

void activity_finalize (void)
{
}

void activity_pulse (void)
{
    activity_write (1);
    activity_t0 = HAL_GetTick ();
}

void activity_update (void)
{
    if (activity_state == 1) {
        if (timesince (activity_t0) >= activity_period) {
            activity_write (0);
        }
    }
}

/*
* vi:tabstop=4 shiftwidth=4 expandtab
*/
