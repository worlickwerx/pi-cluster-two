#include <WProgram.h>
#include <Entropy.h>
#include "canmgr.h"
#include "heartbeat.h"
#include "target_power.h"

const uint32_t heartbeat_period = 1000;// millisec
elapsedMillis heartbeat_since;
int heartbeat_started = 0;

/* If module is powered on simultaneously, compute controlers
 * will synchronize heartbeats and tend to have collision.
 * Introduce some randomness to avoid that.
 * Maximize delay between setup and update to allow entropy
 * to accumulate.
 */
void heartbeat_setup (void)
{
    Entropy.initialize ();
}

void heartbeat_finalize (void)
{
}

void heartbeat_update (void)
{
    if (!heartbeat_started) {
        heartbeat_since = Entropy.random (heartbeat_period);
        heartbeat_started = 1;
    }
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
