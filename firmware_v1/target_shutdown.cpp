#include <WProgram.h>
#include "target_shutdown.h"

const uint8_t target_shutdown_pin = 20;

const uint32_t target_shutdown_period = 100;// millisec
elapsedMillis target_shutdown_since;

uint8_t target_shutdown_state = HIGH;
uint8_t target_shutdown_hold = 0;

void target_shutdown_setup (void)
{
    pinMode (target_shutdown_pin, OUTPUT);
    digitalWriteFast (target_shutdown_pin, target_shutdown_state);
}

void target_shutdown_finalize (void)
{
}

void target_shutdown_pulse (void)
{
    target_shutdown_state = LOW;
    digitalWriteFast (target_shutdown_pin, target_shutdown_state);
    target_shutdown_since -= target_shutdown_since;
}

void target_shutdown_set (uint8_t val)
{
    switch (val) {
        case 0: /* release from shutdown */
            target_shutdown_state = HIGH;
            target_shutdown_hold = 0;
            break;
        case 1: /* hold in shutdown */
            target_shutdown_state = LOW;
            target_shutdown_hold = 1;
            break;
        case 2: /* pulse */
            target_shutdown_state = LOW;
            target_shutdown_hold = 0;
            break;
        default:
            return;
    }
    digitalWriteFast (target_shutdown_pin, target_shutdown_state);
    target_shutdown_since -= target_shutdown_since;
}

void target_shutdown_get (uint8_t *val)
{
    *val = target_shutdown_hold;
}

void target_shutdown_update (void)
{
    if (target_shutdown_since >= target_shutdown_period) {
        if (target_shutdown_state == LOW && !target_shutdown_hold) {
            target_shutdown_state = HIGH;
            digitalWriteFast (target_shutdown_pin, target_shutdown_state);
        }
        target_shutdown_since -= target_shutdown_period;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
