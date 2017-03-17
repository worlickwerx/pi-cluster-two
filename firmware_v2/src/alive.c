#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "alive.h"

const uint16_t alive_pin = GPIO_PIN_12;
const uint16_t alive_period = 200;
uint32_t alive_t0;
uint8_t alive_state = 0;

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

void alive_setup (void)
{
    confpin (alive_pin);

    writepin (alive_pin, alive_state);
    alive_t0 = HAL_GetTick ();

    itm_printf ("Alive: initialized\n");
}

void alive_finalize (void)
{
}

void alive_update (void)
{
    uint32_t t1 = HAL_GetTick ();
    uint32_t elapsed = t1 < alive_t0 ? 0xFFFFFFFF - alive_t0 + t1
                                     : t1 - alive_t0;
    if (elapsed >= alive_period) {
        alive_state = alive_state == 0 ? 1 : 0;
        writepin (alive_pin, alive_state);
        alive_t0 = HAL_GetTick ();
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
