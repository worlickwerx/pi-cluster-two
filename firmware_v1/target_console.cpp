#include <WProgram.h>
#include "target_console.h"

void target_console_setup (void)
{
    Serial1.begin (115200, SERIAL_8N1);
}

void target_console_finalize (void)
{
    Serial1.end ();
}

void target_console_send (uint8_t *buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
        Serial1.write (buf[i]);
}

int target_console_available (void)
{
    return (Serial1.available () > 0) ? 1 : 0;
}

int target_console_recv (uint8_t *buf, int len)
{
    int i = 0;
    int c;
    while (i < len && (c = Serial1.read ()) != -1)
        buf[i++] = c;
    return i;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
