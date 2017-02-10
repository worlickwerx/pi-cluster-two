#include <WProgram.h>
#include "canmgr.h"
#include "heartbeat.h"

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
        canmgr_heartbeat_send ();
        heartbeat_since = 0;
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
