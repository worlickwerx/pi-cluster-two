#include <WProgram.h>
#include "target_power.h"

const uint8_t switch_pin = 23;
uint8_t switch_state = LOW;

const uint8_t shutdown_pin = 20;
uint8_t shutdown_state = HIGH;
const uint32_t shutdown_depress_period = 100; // keypress millisec
elapsedMillis shutdown_depress_since;

const uint8_t sense_pin = 2;
const uint32_t poweroff_timeout = 30000; // shutdown > 30s, turn off
elapsedMillis poweroff_since;
uint8_t poweroff = 0;

void target_power_setup (void)
{
    pinMode (switch_pin, OUTPUT);
    digitalWriteFast (switch_pin, switch_state);
    pinMode (shutdown_pin, OUTPUT);
    digitalWriteFast (shutdown_pin, shutdown_state);
    pinMode (sense_pin, INPUT);
}

void target_power_finalize (void)
{
}

void target_power_get (uint8_t *val)
{
    *val = switch_state == HIGH ? 1 : 0;
}

void target_power_set (uint8_t val)
{
    if (val == 0) { // turn off
        switch_state = LOW;
        digitalWriteFast (switch_pin, switch_state);
        poweroff = 0;
    } else if (val == 1) { // turn on
        switch_state = HIGH;
        digitalWriteFast (switch_pin, switch_state);
        poweroff = 0;
        shutdown_state = HIGH;
        digitalWriteFast (shutdown_pin, shutdown_state);
    } else if (val == 2) { // POWER_OFF keypress (shutdown)
        shutdown_state = LOW;
        shutdown_depress_since = 0;
        digitalWriteFast (shutdown_pin, shutdown_state);
        poweroff = 1;
        poweroff_since = 0;
    } else if (val == 3) { // toggle
        if (switch_state == HIGH)
            target_power_set (2);
        else
            target_power_set (1);
    }
}

void target_power_update (void)
{
    /* timed button press */
    if (shutdown_depress_since >= shutdown_depress_period) {
        if (shutdown_state == LOW) {
            shutdown_state = HIGH;
            digitalWriteFast (shutdown_pin, shutdown_state);
        }
        shutdown_depress_since = 0;
    }
    /* poweroff complete - switch off  */
    if (poweroff == 1 && digitalReadFast (sense_pin) == HIGH) {
        switch_state = LOW;
        digitalWriteFast (switch_pin, switch_state);
        poweroff = 0;
    }
    /* poweroff timed out - switch off */
    if (poweroff_since >= poweroff_timeout) {
        if (poweroff == 1) {
            switch_state = LOW;
            digitalWriteFast (switch_pin, switch_state);
            poweroff = 0;
        }
        poweroff_since = 0;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
