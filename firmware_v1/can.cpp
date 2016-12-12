#include <FlexCAN.h>
#include "can.h"

FlexCAN can0 (125000, 0);

void can0_begin (uint32_t idmask)
{
    CAN_filter_t f;
    f.rtr = 0;
    f.ext = 0;
    f.id = idmask;
    can0.begin (f);
}

void can0_end (void)
{
    can0.end ();
}

int can0_available (void)
{
    return can0.available ();
}

void can0_setfilter (uint32_t idmask, uint8_t n)
{
    CAN_filter_t f;
    f.rtr = 0;
    f.ext = 0;
    f.id = idmask;
    can0.setFilter (f, n);
}

int can0_write (uint32_t id, uint8_t len, uint8_t *buf, uint16_t timeout)
{
    CAN_message_t m;
    m.id = id;
    m.ext = 0;
    m.len = len;
    m.timeout = timeout;
    memcpy (m.buf, buf, len);
    return can0.write (m);
}

int can0_read (uint32_t *id, uint8_t *len, uint8_t *buf)
{
    CAN_message_t m;
    int rc = can0.read (m);
    if (rc) {
        if (id)
            *id = m.id;
        if (len)
            *len = m.len;
        if (buf)
            memcpy (buf, m.buf, m.len);
    }
    return rc;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */

