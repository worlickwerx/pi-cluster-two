#include <WProgram.h>
#include "canmgr.h"
#include "identify.h"
#include "activity.h"
#include "target_reset.h"
#include "target_power.h"

const uint8_t can_addr = 0x10; // get from slot id | 0x10

// on linux: cansend can0 200#DEADBEEF
//   e.g. 200|src

int main (void)
{
    canmgr_setup (can_addr);
    identify_setup ();
    activity_setup ();
    target_reset_setup ();
    target_power_setup ();

    while (1) {
        canmgr_update ();
        identify_update ();
        activity_update ();
        target_reset_update ();
    }

    target_power_finalize ();
    target_reset_finalize ();
    activity_finalize ();
    identify_finalize ();
    canmgr_finalize ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

