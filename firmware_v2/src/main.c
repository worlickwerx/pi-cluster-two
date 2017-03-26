#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "mcu.h"
#include "alive.h"
#include "debug.h"
#include "address.h"
#include "activity.h"
#include "identify.h"
#include "can.h"
#include "power.h"
#include "heartbeat.h"

int main (void)
{
    uint8_t mod, node;

    mcu_setup ();
    alive_setup ();
    address_setup ();
    address_get (&mod, &node);
    activity_setup ();
    identify_setup ();
    power_setup ();
    can_setup (mod, node);
    heartbeat_setup ();

    while (1) {
        //alive_update ();
        can_update ();
        activity_update ();
        identify_update ();
        power_update ();
        heartbeat_update ();
    }

    heartbeat_finalize ();
    can_finalize ();
    power_finalize ();
    identify_finalize ();
    activity_finalize ();
    address_finalize ();
    alive_finalize ();
    mcu_finalize ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
