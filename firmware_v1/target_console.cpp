#include <WProgram.h>
#include "target_console.h"

static uint8_t recv_buf[32768];
static unsigned int head, tail;

static unsigned int postincr (unsigned int *i)
{
    unsigned int last = *i;
    if (++(*i) == sizeof (recv_buf))
        *i = 0;
    return last;
}

void target_console_setup (void)
{
    Serial1.begin (115200, SERIAL_8N1);
    head = tail = 0;
}

void target_console_finalize (void)
{
    Serial1.end ();
}

void target_console_update (void)
{
    while (Serial1.available () > 0) {
        recv_buf[postincr (&head)] = Serial1.read ();
        if (head == tail)
            postincr (&tail);
    }
}

void target_console_send (uint8_t *buf, int len)
{
    int i;
    for (i = 0; i < len; i++)
        Serial1.write (buf[i]);
}

int target_console_available (void)
{
    return (head != tail);
}

int target_console_recv (uint8_t *buf, int len)
{
    int i = 0;
    while (i < len && tail != head)
        buf[i++] = recv_buf[postincr (&tail)];
    return i;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
