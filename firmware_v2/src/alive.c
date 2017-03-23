#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "util.h"
#include "alive.h"

const uint16_t alive_pin = GPIO_PIN_12;
GPIO_TypeDef *alive_port = GPIOB;

const uint16_t alive_period = 200;
uint32_t alive_t0;
uint8_t alive_state = 0;

static void alive_write (uint8_t val)
{
    HAL_GPIO_WritePin (alive_port, alive_pin,
                       val ? GPIO_PIN_RESET : GPIO_PIN_SET);
    alive_state = val;
}

void alive_setup (void)
{
    GPIO_InitTypeDef g;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = alive_pin;
    HAL_GPIO_Init (alive_port, &g);

    alive_write (alive_state);
    alive_t0 = HAL_GetTick ();

    itm_printf ("Alive: initialized\n");
}

void alive_finalize (void)
{
}

void alive_update (void)
{
    if (timesince (alive_t0) >= alive_period) {
        alive_write (!alive_state);
        alive_t0 = HAL_GetTick ();
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
