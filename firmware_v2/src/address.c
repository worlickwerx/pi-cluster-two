#include <stm32f1xx.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "address.h"

const uint16_t na0_pin = GPIO_PIN_0;
const uint16_t na1_pin = GPIO_PIN_1;
const uint16_t na2_pin = GPIO_PIN_2;
const uint16_t na3_pin = GPIO_PIN_3;
GPIO_TypeDef *na_port = GPIOA;

const uint16_t ma0_pin = GPIO_PIN_4;
const uint16_t ma1_pin = GPIO_PIN_5;
const uint16_t ma2_pin = GPIO_PIN_6;
const uint16_t ma3_pin = GPIO_PIN_7;
GPIO_TypeDef *ma_port = GPIOA;

static void config_input (GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef g;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = pin;
    HAL_GPIO_Init (port, &g);
}

void address_setup (void)
{
    uint8_t mod, node;

    __GPIOA_CLK_ENABLE();

    config_input (na_port, na0_pin);
    config_input (na_port, na1_pin);
    config_input (na_port, na2_pin);
    config_input (na_port, na3_pin);

    config_input (ma_port, ma0_pin);
    config_input (ma_port, ma1_pin);
    config_input (ma_port, ma2_pin);
    config_input (ma_port, ma3_pin);

    address_get (&mod, &node);
    itm_printf ("Address: %x,%x\n", mod, node);
}

void address_finalize (void)
{
}

void address_get (uint8_t *mod, uint8_t *node)
{
    uint8_t na, ma;

    na = HAL_GPIO_ReadPin (na_port, na0_pin);
    na |= (HAL_GPIO_ReadPin (na_port, na1_pin) << 1);
    na |= (HAL_GPIO_ReadPin (na_port, na2_pin) << 2);
    na |= (HAL_GPIO_ReadPin (na_port, na3_pin) << 3);

    ma = HAL_GPIO_ReadPin (ma_port, ma0_pin);
    ma |= (HAL_GPIO_ReadPin (ma_port, ma1_pin) << 1);
    ma |= (HAL_GPIO_ReadPin (ma_port, ma2_pin) << 2);
    ma |= (HAL_GPIO_ReadPin (ma_port, ma3_pin) << 3);

    *mod = ma;
    *node = na;
}


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
