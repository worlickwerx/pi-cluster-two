#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "power.h"

const uint16_t power_switch_pin = GPIO_PIN_13;
GPIO_TypeDef *power_switch_port = GPIOB;
uint8_t power_switch_state = 0;

static void power_switch_write (uint8_t val)
{
    HAL_GPIO_WritePin (power_switch_port, power_switch_pin,
                       val ? GPIO_PIN_SET : GPIO_PIN_RESET);
    power_switch_state = val;
}

void power_get (uint8_t *val)
{
    *val = power_switch_state;
}

void power_set (uint8_t val)
{
    power_switch_write (val);
}

void power_setup (void)
{
    GPIO_InitTypeDef g;
    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = power_switch_pin;
    HAL_GPIO_Init (power_switch_port, &g);
    HAL_GPIO_WritePin (power_switch_port, power_switch_pin, GPIO_PIN_RESET);

    itm_printf ("Power: initialized\n");
}

void power_finalize (void)
{
}

void power_update (void)
{
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
