#include <WProgram.h>
#include "target_console.h"

void target_console_setup (void)
{
    Serial1.begin (115200);
}

void target_console_finalize (void)
{
}

void target_console_send (char *buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
        Serial1.write (buf[i]);
}

int target_console_recv (char *buf, int len)
{
    int i = 0;
    while (i < len && Serial1.available () > 0)
        buf[i++] = Serial1.read ();
    return i;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
