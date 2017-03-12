#include <stm32f1xx.h>
#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_gpio.h>
#include "debug.h"
#include "identify.h"

const uint16_t identify_pin = GPIO_PIN_9;
const uint16_t identify_period = 1000;
uint32_t identify_t0;

uint8_t identify_mode = 0; // 0=off, 1=on, 2=blink
uint8_t identify_state = 0;

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

void identify_setup (void)
{
    confpin (identify_pin);

    /* visual self-test */
    writepin (identify_pin, 1);
    HAL_Delay (200);
    writepin (identify_pin, 0);

    itm_printf ("Identify: initialized\n");
}

void identify_finalize (void)
{
}

void identify_set (uint8_t val)
{
    identify_mode = val;
    identify_state = identify_mode ? 1 : 0;
    writepin (identify_pin, identify_state);
    identify_t0 = HAL_GetTick ();
}

void identify_get (uint8_t *val)
{
    *val = identify_mode;
}

void identify_update (void)
{
    if (identify_mode == 2) {
        uint32_t t1 = HAL_GetTick ();
        uint32_t elapsed = t1 < identify_t0 ? 0xFFFFFFFF - identify_t0 + t1
                                            : t1 - identify_t0;
        if (elapsed >= identify_period) {
            identify_state = identify_state == 0 ? 1 : 0;
            writepin (identify_pin, identify_state);
            identify_t0 = HAL_GetTick ();
        }
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
