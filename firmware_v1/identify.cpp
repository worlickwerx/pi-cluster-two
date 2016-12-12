#include <WProgram.h>
#include "identify.h"

const uint8_t identify_pin = 14;

const uint16_t identify_period = 1000; // millisec
elapsedMillis identify_since;

uint8_t identify_enable = 0; // disabled
uint8_t identify_state = LOW; // LED off

void identify_setup (void)
{
    pinMode (identify_pin, OUTPUT);
    digitalWriteFast (identify_pin, identify_state);
}

void identify_finalize (void)
{
}

void identify_set (uint8_t val)
{
    identify_enable = val;
    identify_state = identify_enable ? HIGH : LOW;
    digitalWriteFast (identify_pin, identify_state);
    identify_since -= identify_since;
}

void identify_get (uint8_t *val)
{
    *val = identify_enable;
}

void identify_update (void)
{
    if (identify_since >= identify_period) {
        if (identify_enable) {
            identify_state = (identify_state == LOW) ? HIGH : LOW;
            digitalWriteFast (identify_pin, identify_state);
        }
        identify_since -= identify_period;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
