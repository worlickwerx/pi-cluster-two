#include <WProgram.h>
#include "identify.h"

const uint8_t identify_pin = 14;

const uint16_t identify_period = 1000; // millisec
elapsedMillis identify_since;

uint8_t identify_mode = 0; // 0=off, 1=on, 2=blink
uint8_t identify_state = LOW; // LED off

void identify_setup (void)
{
    pinMode (identify_pin, OUTPUT);

    /* quick test */
    digitalWriteFast (identify_pin, HIGH);
    delay (200);
    digitalWriteFast (identify_pin, LOW);
}

void identify_finalize (void)
{
}

void identify_set (uint8_t val)
{
    identify_mode = val;
    identify_state = identify_mode ? HIGH : LOW;
    digitalWriteFast (identify_pin, identify_state);
    identify_since -= identify_since;
}

void identify_get (uint8_t *val)
{
    *val = identify_mode;
}

void identify_update (void)
{
    if (identify_since >= identify_period) {
        if (identify_mode == 2) {
            identify_state = (identify_state == LOW) ? HIGH : LOW;
            digitalWriteFast (identify_pin, identify_state);
        }
        identify_since -= identify_period;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
