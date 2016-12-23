#include <WProgram.h>
#include "target_shutdown.h"

const uint8_t target_shutdown_pin = 20;

const uint32_t target_shutdown_period = 100;// millisec
elapsedMillis target_shutdown_since;

uint8_t target_shutdown_state = HIGH;

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

void target_shutdown_update (void)
{
    if (target_shutdown_since >= target_shutdown_period) {
        if (target_shutdown_state == LOW) {
            target_shutdown_state = HIGH;
            digitalWriteFast (target_shutdown_pin, target_shutdown_state);
        }
        target_shutdown_since -= target_shutdown_period;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
