#include <WProgram.h>
#include "target_power.h"

const uint8_t switch_pin = 23;
uint8_t switch_state = LOW;

// /etc/systemd/logind.conf: HandlePowerKey=poweroff
// dtoverlay shutdown
// to pi gpio 20 (pin 38)
const uint8_t shutdown_pin = 20;
uint8_t shutdown_state = HIGH;
const uint32_t shutdown_depress_period = 100; // keypress millisec
elapsedMillis shutdown_depress_since;

// dtoverlay poweroff
// to pi gpio 26 (pin 37)
const uint8_t sense_pin = 2;
const uint32_t shutdown_timeout = 30000; // shutdown > 30s, turn off
elapsedMillis shutdown_since;
uint8_t shutdown = 0;

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
    if (switch_state == LOW)
        *val = 0;
    else if (!shutdown)
        *val = 1;
    else /* shutting down */
        *val = 2;
}

void target_power_set (uint8_t val)
{
    if (val == 0) { // turn off
        switch_state = LOW;
        digitalWriteFast (switch_pin, switch_state);
        shutdown = 0;
    } else if (val == 1) { // turn on
        switch_state = HIGH;
        digitalWriteFast (switch_pin, switch_state);
        shutdown = 0;
        shutdown_state = HIGH;
        digitalWriteFast (shutdown_pin, shutdown_state);
    } else if (val == 2) { // POWER_OFF keypress (shutdown)
        shutdown_state = LOW;
        shutdown_depress_since = 0;
        digitalWriteFast (shutdown_pin, shutdown_state);
        shutdown = 1;
        shutdown_since = 0;
    } else if (val == 3) { // toggle
        if (switch_state == LOW)
            target_power_set (1);
        else if (!shutdown)
            target_power_set (2);
        else
            target_power_set (0);
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
    /* shutdown complete - switch off  */
    if (shutdown == 1 && digitalReadFast (sense_pin) == HIGH) {
        switch_state = LOW;
        digitalWriteFast (switch_pin, switch_state);
        shutdown = 0;
    }
    /* shutdown timed out - switch off */
    if (shutdown_since >= shutdown_timeout) {
        if (shutdown == 1) {
            switch_state = LOW;
            digitalWriteFast (switch_pin, switch_state);
            shutdown = 0;
        }
        shutdown_since = 0;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
