#include <WProgram.h>
#include "address.h"

const uint8_t addr0_pin = 16;
const uint8_t addr1_pin = 17;
const uint8_t addr2_pin = 18;
const uint8_t addr3_pin = 19;

void address_setup (void)
{
    pinMode (addr0_pin, INPUT_PULLUP);
    pinMode (addr1_pin, INPUT_PULLUP);
    pinMode (addr2_pin, INPUT_PULLUP);
    pinMode (addr3_pin, INPUT_PULLUP);
}

void address_finalize (void)
{
}

void address_get (uint8_t *val)
{
    uint8_t addr;

    addr = digitalRead (addr0_pin);
    addr |= (digitalRead (addr1_pin) << 1);
    addr |= (digitalRead (addr2_pin) << 2);
    addr |= (digitalRead (addr3_pin) << 3);

    *val = addr;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
