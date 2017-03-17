#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include <stdio.h>
#include <stdarg.h>
#include "debug.h"

/* Borrowed from ITM_SendChar() in system/cmsis/core_cm3.h.
 */
static int itm_enabled (void)
{
    if (((ITM->TCR & ITM_TCR_ITMENA_Msk) != 0UL) &&  /* ITM enabled */
        ((ITM->TER & 1UL               ) != 0UL)   ) /* ITM Port #0 enabled */
        return 1;
    return 0;
}

void itm_puts (const char *s)
{
    if (!itm_enabled ())
        return;
    while (*s)
        ITM_SendChar (*s++);
}

void itm_vprintf (const char *fmt, va_list ap)
{
    if (!itm_enabled ())
        return;
    char buf[80];
    vsnprintf (buf, sizeof (buf), fmt, ap);
    itm_puts (buf);
}

void itm_printf (const char *fmt, ...)
{
    if (!itm_enabled ())
        return;
    va_list ap;
    va_start (ap, fmt);
    itm_vprintf (fmt, ap);
    va_end (ap);
}

void itm_fatal (const char *msg, const char *file, int line)
{
    itm_printf ("fatal error (%s::%d): %s\n", file, line, msg);
    while (1)
        ;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
