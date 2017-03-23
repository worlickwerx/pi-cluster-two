#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "power.h"
#include "util.h"

typedef enum { OFF, ON } power_state_t;
const uint16_t power_switch_pin = GPIO_PIN_13;
GPIO_TypeDef *power_switch_port = GPIOB;
power_state_t power_switch_state = OFF;

typedef enum { UNPRESSED, PRESSED } button_state_t;
const uint16_t power_button_pin = GPIO_PIN_14;
GPIO_TypeDef *power_button_port = GPIOB;
button_state_t power_button_state = UNPRESSED;
const uint32_t power_button_period = 100; // msec
uint32_t power_button_t0;

typedef enum { HALTED, RUNNING } sense_state_t;
const uint32_t power_sense_pin = GPIO_PIN_15;
GPIO_TypeDef *power_sense_port = GPIOB;

const uint32_t linux_shutdown_timeout = 30000; // turn off after 30s
uint32_t linux_shutdown_t0;
uint8_t linux_shutdown_inprogress = 0;

static void power_switch_write (power_state_t val)
{
    HAL_GPIO_WritePin (power_switch_port, power_switch_pin,
                       val == ON ? GPIO_PIN_SET : GPIO_PIN_RESET);
    power_switch_state = val;
}

static void power_button_write (button_state_t val)
{
    HAL_GPIO_WritePin (power_button_port, power_button_pin,
                       val == PRESSED ? GPIO_PIN_RESET : GPIO_PIN_SET);
    power_button_state = val;
}

static sense_state_t power_sense_read (void)
{
    if (HAL_GPIO_ReadPin (power_sense_port, power_sense_pin) == GPIO_PIN_SET)
        return HALTED;
    else
        return RUNNING;
}

void power_get (uint8_t *val)
{
    if (power_switch_state == 0)
        *val = 0;
    else if (!linux_shutdown_inprogress)
        *val = 1;
    else /* shutting down */
        *val = 2;
}

void power_set (uint8_t val)
{
    if (val == 0) {
        itm_printf ("Power: powering off\n");
        power_switch_write (OFF);
        power_button_write (UNPRESSED);
        linux_shutdown_inprogress = 0;
    } else if (val == 1) {
        itm_printf ("Power: powering on\n");
        power_switch_write (ON);
        power_button_write (UNPRESSED);
        linux_shutdown_inprogress = 0;
    } else if (val == 2) {
        itm_printf ("Power: initiating Linux shutdown\n");
        power_button_write (PRESSED);
        power_button_t0 = linux_shutdown_t0 = HAL_GetTick ();
        linux_shutdown_inprogress = 1;
    } else if (val == 3) {
        if (power_switch_state == OFF)
            power_set (1);
        else if (!linux_shutdown_inprogress)
            power_set (2);
        else
            power_set (0);
    }
}

void power_setup (void)
{
    GPIO_InitTypeDef g;

    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = power_switch_pin;
    HAL_GPIO_Init (power_switch_port, &g);
    power_switch_write (power_switch_state);

    g.Mode = GPIO_MODE_OUTPUT_PP;
    g.Pull = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = power_button_pin;
    HAL_GPIO_Init (power_button_port, &g);
    power_button_write (power_button_state);

    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_PULLDOWN;
    g.Speed = GPIO_SPEED_HIGH;
    g.Pin = power_sense_pin;
    HAL_GPIO_Init (power_sense_port, &g);

    itm_printf ("Power: initialized\n");
}

void power_finalize (void)
{
}

void power_update (void)
{
    if (power_button_state == PRESSED
            && timesince (power_button_t0) >= power_button_period) {
        power_button_write (UNPRESSED);
    }
    if (linux_shutdown_inprogress) {
        if (power_sense_read () == HALTED) {
            itm_printf ("Power: shutdown completed, powering off\n");
            power_switch_write (OFF);
            linux_shutdown_inprogress = 0;
        }
        if (timesince (linux_shutdown_t0) >= linux_shutdown_timeout) {
            itm_printf ("Power: shutdown timeout of %ds expired, powering off\n",
                        linux_shutdown_timeout / 1000);
            power_switch_write (OFF);
            linux_shutdown_inprogress = 0;
        }
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
