#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "util.h"
#include "identify.h"

const uint16_t identify_pin = GPIO_PIN_9;
GPIO_TypeDef *identify_port = GPIOB;

const uint16_t identify_period = 1000;
uint32_t identify_t0;

uint8_t identify_mode = 0; // 0=off, 1=on, 2=blink
uint8_t identify_state = 0;

static void identify_write (uint8_t val)
{
    HAL_GPIO_WritePin (identify_port, identify_pin,
                       val ? GPIO_PIN_SET : GPIO_PIN_RESET);
    identify_state = val; 
}

void identify_setup (void)
{
    GPIO_InitTypeDef g;

    __GPIOB_CLK_ENABLE();

    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = identify_pin;
    HAL_GPIO_Init (identify_port, &g);

    /* visual self-test */
    identify_write (1);
    HAL_Delay (200);
    identify_write (0);

    itm_printf ("Identify: initialized\n");
}

void identify_finalize (void)
{
}

void identify_set (uint8_t val)
{
    identify_mode = val;
    identify_write (identify_mode ? 1 : 0);
    if (identify_mode == 2)
        identify_t0 = HAL_GetTick ();
}

void identify_get (uint8_t *val)
{
    *val = identify_mode;
}

void identify_update (void)
{
    if (identify_mode == 2) {
        if (timesince (identify_t0) >= identify_period) {
            identify_write (!identify_state);
            identify_t0 = HAL_GetTick ();
        }
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
