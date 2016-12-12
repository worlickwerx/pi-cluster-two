#include <WProgram.h>
#include "target_power.h"

const uint8_t target_power_pin = 23;

uint8_t target_power_state = LOW;

void target_power_setup (void)
{
    pinMode (target_power_pin, OUTPUT);
    digitalWriteFast (target_power_pin, target_power_state);
}

void target_power_finalize (void)
{
}

void target_power_set (uint8_t val)
{
    target_power_state = val ? HIGH : LOW;
    digitalWriteFast (target_power_pin, target_power_state);
}

void target_power_get (uint8_t *val)
{
    *val = target_power_state == LOW ? 0 : 1;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
