#include <WProgram.h>
#include "canmgr.h"
#include "heartbeat.h"
#include "target_power.h"

const uint32_t heartbeat_period = 1000;// millisec
elapsedMillis heartbeat_since;

void heartbeat_setup (void)
{
}

void heartbeat_finalize (void)
{
}

void heartbeat_update (void)
{
    if (heartbeat_since >= heartbeat_period) {
        uint8_t data[8];
        target_power_get (&data[0]);
        canmgr_heartbeat_send (data, 1);
        heartbeat_since = 0;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
