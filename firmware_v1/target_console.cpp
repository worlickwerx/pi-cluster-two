#include <WProgram.h>
#include "target_console.h"

static uint8_t recv_buf[32768];
static unsigned int head, tail, tail_hist, tail_cursor;

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
    head = tail = tail_hist = tail_cursor = 0;
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
        if (head == tail_hist)
            postincr (&tail_hist);
        if (head == tail_cursor)
            postincr (&tail_cursor);
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

void target_console_reset (void)
{
    tail = head;
}

int target_console_recv (uint8_t *buf, int len)
{
    int i = 0;
    while (i < len && tail != head)
        buf[i++] = recv_buf[postincr (&tail)];
    return i;
}

int target_console_history_next (uint8_t *buf, int len)
{
    int i = 0;
    while (i < len && tail_cursor != head)
        buf[i++] = recv_buf[postincr (&tail_cursor)];
    return i;
}

void target_console_history_reset (void)
{
    tail_cursor = tail_hist;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
