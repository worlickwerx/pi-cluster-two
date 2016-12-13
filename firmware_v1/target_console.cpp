#include <WProgram.h>
#include "target_console.h"

static console_receiver_f console_receiver = NULL;
static void *console_receiver_arg = NULL;

void target_console_setup (void)
{
    Serial1.begin (115200);
}

void target_console_finalize (void)
{
}

void target_console_update (void)
{
    char buf[4];
    int len = 0;
    while (len < 4 && Serial1.available () > 0) {
        buf[len++] = Serial1.read ();
    }
    if (len > 0 && console_receiver)
        console_receiver (buf, len, console_receiver_arg);
}

void target_console_set_receiver (console_receiver_f cb, void *arg)
{
    console_receiver = cb;
    console_receiver_arg = arg;
}

void target_console_send (char *buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
        Serial1.write (buf[i]);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
