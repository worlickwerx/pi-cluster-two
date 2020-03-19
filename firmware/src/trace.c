/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <libopencm3/stm32/dbgmcu.h>
#include <libopencm3/cm3/itm.h>

#include "trace.h"

/* Print to SWO trace output.
 * Access with "stlink-trace -c 72".
 * If ITM port is not enabled (no debugger is attached), do nothing.
 */
void trace_puts (const char *s)
{
	if ((ITM_TCR & ITM_TCR_ITMENA) && ITM_TER[0] & 1) {
		while (s && *s) {
			while (!(ITM_STIM8(0) & ITM_STIM_FIFOREADY))
				;
			ITM_STIM8(0) = *s++;
		}
	}
}
