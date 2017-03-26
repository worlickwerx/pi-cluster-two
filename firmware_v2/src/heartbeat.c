#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "can.h"
#include "power.h"
#include "util.h"

uint32_t heartbeat_period = 1000;
uint32_t heartbeat_t0;

/* first heatbeat will be in (heartbeat_period - skew) milliseconds.
 * Set skew either randomly or based on L-CAN address so that
 * compute boards that are powered up simultaneously won't be likely
 * to have heartbeat collisions.
 */
void heartbeat_setup (int skew)
{
    itm_printf ("Heartbeat: initialized\n");
    heartbeat_t0 = HAL_GetTick () + skew;
}

void heartbeat_finalize (void)
{
}

void heartbeat_update (void)
{
    if (timesince (heartbeat_t0) >= heartbeat_period) {
        uint8_t data[8];
        power_get (&data[0]);
        can_heartbeat_send (data, 1);
        heartbeat_t0 = HAL_GetTick ();
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
