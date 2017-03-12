#include <stm32f1xx.h>
#include <stm32f1xx_hal_gpio.h>
#include "debug.h"
#include "address.h"

const uint16_t na0_pin = GPIO_PIN_0;
const uint16_t na1_pin = GPIO_PIN_1;
const uint16_t na2_pin = GPIO_PIN_2;
const uint16_t na3_pin = GPIO_PIN_3;

const uint16_t ma0_pin = GPIO_PIN_4;
const uint16_t ma1_pin = GPIO_PIN_5;
const uint16_t ma2_pin = GPIO_PIN_6;
const uint16_t ma3_pin = GPIO_PIN_7;

/* Configure pin on GPIOA as input
 */
static void confpin (uint16_t pin)
{
    GPIO_InitTypeDef g;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = pin;
    HAL_GPIO_Init (GPIOA, &g);
}

static uint8_t readpin (uint16_t pin)
{
    return HAL_GPIO_ReadPin (GPIOA, pin);
}

void address_setup (void)
{
    uint8_t mod, node;

    confpin (na0_pin);
    confpin (na1_pin);
    confpin (na2_pin);
    confpin (na3_pin);

    confpin (ma0_pin);
    confpin (ma1_pin);
    confpin (ma2_pin);
    confpin (ma3_pin);

    address_get (&mod, &node);
    itm_printf ("Address: %x,%x\n", mod, node);
}

void address_finalize (void)
{
}

void address_get (uint8_t *mod, uint8_t *node)
{
    uint8_t na, ma;

    na = readpin (na0_pin);
    na |= (readpin (na1_pin) << 1);
    na |= (readpin (na2_pin) << 2);
    na |= (readpin (na3_pin) << 3);

    ma = readpin (ma0_pin);
    ma |= (readpin (ma1_pin) << 1);
    ma |= (readpin (ma2_pin) << 2);
    ma |= (readpin (ma3_pin) << 3);

    *mod = ma;
    *node = na;
}


/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
