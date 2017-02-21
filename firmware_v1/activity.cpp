#include <WProgram.h>
#include "activity.h"

const uint8_t activity_pin = 15;

const uint8_t activity_period = 10;// millisec
elapsedMillis activity_since;

uint8_t activity_state = LOW;

void activity_setup (void)
{
    pinMode (activity_pin, OUTPUT);

    /* quick test */
    digitalWriteFast (activity_pin, HIGH);
    delay(200);
    digitalWriteFast (activity_pin, LOW);
}

void activity_finalize (void)
{
}

void activity_pulse (void)
{
    activity_state = HIGH;
    digitalWriteFast (activity_pin, activity_state);
    activity_since -= activity_since;
}

void activity_update (void)
{
    if (activity_since >= activity_period) {
        if (activity_state == HIGH) {
            activity_state = LOW;
            digitalWriteFast (activity_pin, activity_state);
        }
        activity_since -= activity_period;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
