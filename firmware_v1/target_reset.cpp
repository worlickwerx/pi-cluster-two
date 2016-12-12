#include <WProgram.h>
#include "target_reset.h"

const uint8_t target_reset_pin = 22;

const uint32_t target_reset_period = 500;// millisec
elapsedMillis target_reset_since;

uint8_t target_reset_state = HIGH;
uint8_t target_reset_hold = 0;

void target_reset_setup (void)
{
    pinMode (target_reset_pin, OUTPUT);
    digitalWriteFast (target_reset_pin, target_reset_state);
}

void target_reset_finalize (void)
{
}

void target_reset_pulse (void)
{
    target_reset_state = LOW;
    digitalWriteFast (target_reset_pin, target_reset_state);
    target_reset_since -= target_reset_since;
}

void target_reset_set (uint8_t val)
{
    switch (val) {
        case 0: /* release from reset */
            target_reset_state = HIGH;
            target_reset_hold = 0;
            break;
        case 1: /* hold in reset */
            target_reset_state = LOW;
            target_reset_hold = 1;
            break;
        case 2: /* pulse */
            target_reset_state = LOW;
            target_reset_hold = 0;
            break;
        default:
            return;
    }
    digitalWriteFast (target_reset_pin, target_reset_state);
    target_reset_since -= target_reset_since;
}

void target_reset_get (uint8_t *val)
{
    *val = target_reset_hold;
}

void target_reset_update (void)
{
    if (target_reset_since >= target_reset_period) {
        if (target_reset_state == LOW && !target_reset_hold) {
            target_reset_state = HIGH;
            digitalWriteFast (target_reset_pin, target_reset_state);
        }
        target_reset_since -= target_reset_period;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
