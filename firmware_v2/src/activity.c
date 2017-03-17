#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "activity.h"

const uint16_t activity_pin = GPIO_PIN_5;

const uint8_t activity_period = 10; // millsec
uint32_t activity_t0;

uint8_t activity_state = 0;

/* Configure pin on GPIOB as output
 */
static void confpin (uint16_t pin)
{
    GPIO_InitTypeDef g;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = pin;
    HAL_GPIO_Init (GPIOB, &g);
}

static void writepin (uint16_t pin, uint8_t val)
{
    HAL_GPIO_WritePin (GPIOB, pin, val ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void activity_setup (void)
{
    confpin (activity_pin);

    /* visual self-test */
    writepin (activity_pin, 1);
    HAL_Delay (200);
    writepin (activity_pin, 0);

    itm_printf ("Activity: initialized\n");
}

void activity_finalize (void)
{
}

void activity_pulse (void)
{
    activity_state = 1;
    writepin (activity_pin, activity_state);
    activity_t0 = HAL_GetTick ();
}

void activity_update (void)
{
    if (activity_state == 1) {
        uint32_t t1 = HAL_GetTick ();
        uint32_t elapsed = t1 < activity_t0 ? 0xFFFFFFFF - activity_t0 + t1
                                            : t1 - activity_t0;
        if (elapsed >= activity_period) {
            activity_state = 0;
            writepin (activity_pin, activity_state);
        }
    }
}

/*
* vi:tabstop=4 shiftwidth=4 expandtab
*/
