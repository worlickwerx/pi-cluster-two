#include <WProgram.h>
#include "canmgr.h"
#include "identify.h"
#include "address.h"
#include "activity.h"
#include "target_reset.h"
#include "target_shutdown.h"
#include "target_power.h"
#include "target_console.h"

int main (void)
{
    uint8_t addr;

    identify_setup ();
    activity_setup ();
    target_reset_setup ();
    target_shutdown_setup ();
    target_power_setup ();
    target_console_setup ();
    address_setup ();
    address_get (&addr);
    canmgr_setup (addr | 0x10);

    while (1) {
        canmgr_update ();
        identify_update ();
        activity_update ();
        target_reset_update ();
        target_shutdown_update ();
        target_console_update ();
    }

    canmgr_finalize ();
    address_finalize ();
    target_console_finalize ();
    target_power_finalize ();
    target_shutdown_finalize ();
    target_reset_finalize ();
    activity_finalize ();
    identify_finalize ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

