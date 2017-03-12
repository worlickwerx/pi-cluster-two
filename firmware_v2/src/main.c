#include <stm32f1xx.h>

#include <stm32f1xx_hal.h>

#include "mcu.h"
#include "debug.h"
#include "address.h"
#include "activity.h"
#include "identify.h"

int main (void)
{
    uint8_t mod, node;

    mcu_setup ();
    address_setup ();
    address_get (&mod, &node);
    activity_setup ();
    identify_setup ();

    identify_set (2);

    while (1) {
        activity_update ();
        identify_update ();
        if (HAL_GetTick () % 500 == 0)
            activity_pulse ();
    }

    identify_finalize ();
    activity_finalize ();
    address_finalize ();
    mcu_finalize ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
