/* SPDX-License-Identifier: GPL-3.0-or-later */

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "string.h"

#include "canbus.h"
#include "matrix.h"
#include "trace.h"
#include "address.h"
#include "canmsg_v2.h"
#include "power.h"
#include "serial.h"

#include "canservices.h"

/* State for console connection
 */
struct console {
    uint8_t dstaddr;
    uint8_t object;
    bool connected;
    TaskHandle_t task;
};

static struct console console;
uint8_t srcaddr;


static void v2_trace (const struct canmsg_v2 *msg)
{
    trace_printf ("%x->%x %s %s [%d bytes]\n",
                  msg->src,
                  msg->dst,
                  canmsg_v2_typestr (msg),
                  canmsg_v2_objstr (msg),
                  msg->dlen);
}

/* Assume native little endian (ARM can be either, but usually this).
 * Network byte order is big endian.
 */
static uint16_t htons (uint16_t val)
{
    return (val << 8) | (val >> 8);
}

static int send_v2 (const struct canmsg_v2 *msg)
{
    struct canmsg_raw raw;

    if (canmsg_v2_encode (msg, &raw) < 0)
        return -1;
    v2_trace (msg);
    return canbus_send (&raw);
}

static void send_v2_nak (const struct canmsg_v2 *request)
{
    struct canmsg_v2 msg = *request;

    if (request->type == CANMSG_V2_TYPE_WNA)
        return;

    msg.type = CANMSG_V2_TYPE_NAK;
    msg.dst = msg.src;
    msg.src = srcaddr;
    msg.dlen = 0;

    if (send_v2 (&msg) < 0)
        trace_printf ("can-rx: error sending NAK response\n");
}

static void canservices_v2_echo (const struct canmsg_v2 *request)
{
    struct canmsg_v2 msg = *request;

    if (request->type != CANMSG_V2_TYPE_WO) {
        send_v2_nak (request);
        return;
    }

    msg.type = CANMSG_V2_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = srcaddr;

    if (send_v2 (&msg) < 0)
        trace_printf ("can-rx: error sending echo response\n");
}

static void canservices_v2_power (const struct canmsg_v2 *request)
{
    struct canmsg_v2 msg = *request;

    if (request->type == CANMSG_V2_TYPE_WO) {
        if (request->dlen != 1)
            goto error;
        if (request->data[0] == 0)
            power_set_state (false);
        else if (request->data[0] == 1)
            power_set_state (true);
        else
            goto error;
        msg.type = CANMSG_V2_TYPE_ACK;
        msg.dst = msg.src;
        msg.src = srcaddr;
        msg.dlen = 0;
    }
    else if (request->type == CANMSG_V2_TYPE_RO) {
        msg.type = CANMSG_V2_TYPE_ACK;
        msg.dst = msg.src;
        msg.src = srcaddr;
        msg.dlen = 1;
        msg.data[0] = power_get_state () ? 1 : 0;
    }
    else
        goto error;

    if (send_v2 (&msg) < 0)
        trace_printf ("can-rx: error sending power response\n");
    return;
error:
    send_v2_nak (request);
}

static void canservices_v2_power_measure (const struct canmsg_v2 *request)
{
    struct canmsg_v2 msg = *request;
    uint16_t ma;

    if (request->type != CANMSG_V2_TYPE_RO)
        goto error;
    if (request->dlen != 0)
        goto error;

    msg.type = CANMSG_V2_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = srcaddr;

    power_get_measurements (&ma, NULL);
    msg.dlen = 2;
    *(uint16_t *)msg.data = htons (ma);

    if (send_v2 (&msg) < 0)
        trace_printf ("can-rx: error sending power measure response\n");
    return;
error:
    send_v2_nak (request);
}

static void canservices_v2_consoleconn (const struct canmsg_v2 *request)
{
    struct canmsg_v2 msg = *request;

    if (request->type != CANMSG_V2_TYPE_WO)
        goto error;
    if (request->dlen != 1)
        goto error;

    serial_rx_enable ();

    console.dstaddr = request->src;
    console.object = request->data[0];
    console.connected = true;
    vTaskResume (console.task);

    msg.type = CANMSG_V2_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = srcaddr;
    msg.dlen = 0;
    if (send_v2 (&msg) < 0)
        trace_printf ("can-rx: error sending console connect response\n");
    return;
error:
    send_v2_nak (request);
}

static void canservices_v2_consoledisc (const struct canmsg_v2 *request)
{
    struct canmsg_v2 msg = *request;

    if (request->type != CANMSG_V2_TYPE_WO)
        goto error;
    if (request->dlen != 1)
        goto error;
    if (!console.connected || console.dstaddr != request->src
                           || console.object != request->data[0])
        goto error;

    serial_rx_disable ();

    console.connected = false;
    vTaskSuspend (console.task);

    msg.type = CANMSG_V2_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = srcaddr;
    msg.dlen = 0;

    if (send_v2 (&msg) < 0)
        trace_printf ("can-rx: error sending console disconnect response\n");
    return;
error:
    send_v2_nak (request);
}

/* can -> serial */
static void canservices_v2_consolerecv (const struct canmsg_v2 *request)
{
    struct canmsg_v2 msg = *request;

    if (request->type != CANMSG_V2_TYPE_DAT)
        goto error;
    if (!console.connected)
        goto error;

    if (serial_send (request->data, request->dlen, 0) < request->dlen)
        goto error;

    msg.type = CANMSG_V2_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = srcaddr;
    msg.dlen = 0;

    if (send_v2 (&msg) < 0)
        trace_printf ("can-rx: error sending console data response\n");
    return;
error:
    send_v2_nak (request);
}

/* serial -> can */
static void canservices_console_task (void *arg __attribute((unused)))
{
    struct canmsg_v2 msg;

    memset (&msg, 0, sizeof (msg));

    msg.type = CANMSG_V2_TYPE_DAT;
    msg.src = srcaddr;
    msg.pri = 1;

    for (;;) {
        msg.dlen = serial_recv (msg.data, sizeof (msg.data), 1);

        if (msg.dlen > 0) {
            msg.dst = console.dstaddr;
            msg.object = console.object;

            if (send_v2 (&msg) < 0)
                trace_printf ("can-console: error sending console data\n");
            else
                vTaskSuspend (console.task); // suspend task, pending ACK
        }
    }
}

static void canservices_rx_task (void *arg __attribute((unused)))
{
    struct canmsg_raw raw;
    struct canmsg_v2 msg;

    for (;;) {
        if (canbus_recv (&raw, -1) == 0) {
            if (canmsg_v2_decode (&raw, &msg) < 0) {
                trace_printf ("can-rx: error decoding received message\n");
                continue;
            }

            v2_trace (&msg);

            switch (msg.type) {
                case CANMSG_V2_TYPE_WO:
                case CANMSG_V2_TYPE_RO:
                case CANMSG_V2_TYPE_WNA:
                case CANMSG_V2_TYPE_DAT:
                    switch (msg.object) {
                        case CANMSG_V2_OBJ_ECHO:
                            canservices_v2_echo (&msg);
                            break;
                        case CANMSG_V2_OBJ_POWER:
                            canservices_v2_power (&msg);
                            break;
                        case CANMSG_V2_OBJ_POWER_MEASURE:
                            canservices_v2_power_measure (&msg);
                            break;
                        case CANMSG_V2_OBJ_CONSOLECONN:
                            canservices_v2_consoleconn (&msg);
                            break;
                        case CANMSG_V2_OBJ_CONSOLEDISC:
                            canservices_v2_consoledisc (&msg);
                            break;
                        case CANMSG_V2_OBJ_CONSOLERECV:
                            canservices_v2_consolerecv (&msg);
                            break;
                        default: // unknown object id
                            send_v2_nak (&msg);
                            break;
                    }
                    break;
                case CANMSG_V2_TYPE_ACK:
                    if (console.connected && msg.object == console.object)
                        vTaskResume (console.task);
                    break;
                case CANMSG_V2_TYPE_NAK:
                case CANMSG_V2_TYPE_SIG:
                    break;
            }
        }
    }
}

void canservices_init (void)
{
    srcaddr = address_get () | CANMSG_V2_ADDR_CONTROL;

    canbus_filter_set (0, (srcaddr << CANMSG_V2_DST_SHIFT), CANMSG_V2_DST_MASK);

    xTaskCreate (canservices_rx_task,
                 "canrx",
                 400,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 NULL);

    xTaskCreate (canservices_console_task,
                 "cancon",
                 400,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 &console.task);
    vTaskSuspend (console.task);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
