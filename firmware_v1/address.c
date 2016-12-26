#include <WProgram.h>
#include "address.h"

const uint8_t na0_pin = 16;
const uint8_t na1_pin = 17;
const uint8_t na2_pin = 18;
const uint8_t na3_pin = 19;

const uint8_t ma0_pin = 9;
const uint8_t ma1_pin = 8;
const uint8_t ma2_pin = 7;
const uint8_t ma3_pin = 6;

void address_setup (void)
{
    pinMode (na0_pin, INPUT_PULLUP);
    pinMode (na1_pin, INPUT_PULLUP);
    pinMode (na2_pin, INPUT_PULLUP);
    pinMode (na3_pin, INPUT_PULLUP);

    pinMode (ma0_pin, INPUT_PULLUP);
    pinMode (ma1_pin, INPUT_PULLUP);
    pinMode (ma2_pin, INPUT_PULLUP);
    pinMode (ma3_pin, INPUT_PULLUP);
}

void address_finalize (void)
{
}

void address_get (uint8_t *mod, uint8_t *node)
{
    uint8_t na, ma;

    na = digitalRead (na0_pin);
    na |= (digitalRead (na1_pin) << 1);
    na |= (digitalRead (na2_pin) << 2);
    na |= (digitalRead (na3_pin) << 3);

    ma = digitalRead (ma0_pin);
    ma |= (digitalRead (ma1_pin) << 1);
    ma |= (digitalRead (ma2_pin) << 2);
    ma |= (digitalRead (ma3_pin) << 3);

    *node = na;
    *mod = ma;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
