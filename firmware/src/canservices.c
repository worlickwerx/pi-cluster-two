/************************************************************\
 * Copyright 2023 Jim Garlick <garlick.jim@gmail.com>
 *
 * This file is part of the pi cluster II project
 * https://github.com/worlickwerx/pi-cluster-two
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
\************************************************************/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "string.h"

#include "canbus.h"
#include "matrix.h"
#include "trace.h"
#include "address.h"
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


static void trace_msg (const struct canmsg *msg)
{
    trace_printf ("%x->%x %x %x [%d bytes]\n",
                  msg->src,
                  msg->dst,
                  msg->type,
                  msg->object,
                  msg->dlen);
}

static int send_msg (const struct canmsg *msg)
{
    trace_msg (msg);
    return canbus_send (msg);
}

static void send_nak (const struct canmsg *request)
{
    struct canmsg msg = *request;

    if (request->type == CANMSG_TYPE_WNA)
        return;

    msg.type = CANMSG_TYPE_NAK;
    msg.dst = msg.src;
    msg.src = srcaddr;
    msg.dlen = 0;

    if (send_msg (&msg) < 0)
        trace_printf ("can-rx: error sending NAK response\n");
}

static void canservices_echo (const struct canmsg *request)
{
    struct canmsg msg = *request;

    if (request->type != CANMSG_TYPE_WO) {
        send_nak (request);
        return;
    }

    msg.type = CANMSG_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = srcaddr;

    if (send_msg (&msg) < 0)
        trace_printf ("can-rx: error sending echo response\n");
}

static void canservices_power (const struct canmsg *request)
{
    struct canmsg msg = *request;

    if (request->type == CANMSG_TYPE_WO) {
        if (request->dlen != 1)
            goto error;
        if (request->data[0] == 0)
            power_set_state (false); // can sleep
        else if (request->data[0] == 1)
            power_set_state (true); // can sleep
        else
            goto error;
        msg.type = CANMSG_TYPE_ACK;
        msg.dst = msg.src;
        msg.src = srcaddr;
        msg.dlen = 0;
    }
    else if (request->type == CANMSG_TYPE_RO) {
        msg.type = CANMSG_TYPE_ACK;
        msg.dst = msg.src;
        msg.src = srcaddr;
        msg.dlen = 1;
        msg.data[0] = power_get_state () ? 1 : 0;
    }
    else
        goto error;

    if (send_msg (&msg) < 0)
        trace_printf ("can-rx: error sending power response\n");
    return;
error:
    send_nak (request);
}

static void canservices_shutdown (const struct canmsg *request)
{
    struct canmsg msg = *request;

    if (request->type == CANMSG_TYPE_WO) {
        if (request->dlen != 0)
            goto error;
        power_shutdown ();
        msg.type = CANMSG_TYPE_ACK;
        msg.dst = msg.src;
        msg.src = srcaddr;
        msg.dlen = 0;
    }
    else if (request->type == CANMSG_TYPE_RO) {
        msg.type = CANMSG_TYPE_ACK;
        msg.dst = msg.src;
        msg.src = srcaddr;
        msg.dlen = 1;
        msg.data[0] = power_is_shutdown () ? 1 : 0;
    }
    else
        goto error;

    if (send_msg (&msg) < 0)
        trace_printf ("can-rx: error sending halt response\n");
    return;
error:
    send_nak (request);
}

static void canservices_consoleconn (const struct canmsg *request)
{
    struct canmsg msg = *request;

    if (request->type != CANMSG_TYPE_WO)
        goto error;
    if (request->dlen != 1)
        goto error;

    serial_rx_enable ();

    console.dstaddr = request->src;
    console.object = request->data[0];
    console.connected = true;
    vTaskResume (console.task);

    msg.type = CANMSG_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = srcaddr;
    msg.dlen = 0;
    if (send_msg (&msg) < 0)
        trace_printf ("can-rx: error sending console connect response\n");
    return;
error:
    send_nak (request);
}

static void canservices_consoledisc (const struct canmsg *request)
{
    struct canmsg msg = *request;

    if (request->type != CANMSG_TYPE_WO)
        goto error;
    if (request->dlen != 1)
        goto error;
    if (!console.connected || console.dstaddr != request->src
                           || console.object != request->data[0])
        goto error;

    serial_rx_disable ();

    console.connected = false;
    vTaskSuspend (console.task);

    msg.type = CANMSG_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = srcaddr;
    msg.dlen = 0;

    if (send_msg (&msg) < 0)
        trace_printf ("can-rx: error sending console disconnect response\n");
    return;
error:
    send_nak (request);
}

/* can -> serial */
static void canservices_consolerecv (const struct canmsg *request)
{
    struct canmsg msg = *request;

    if (request->type != CANMSG_TYPE_DAT)
        goto error;
    if (!console.connected)
        goto error;

    if (serial_send (request->data, request->dlen, 0) < request->dlen)
        goto error;

    if (request->eot) {
        msg.type = CANMSG_TYPE_ACK;
        msg.dst = msg.src;
        msg.src = srcaddr;
        msg.dlen = 0;
        msg.eot = 0;
        msg.seq = 0;
        if (send_msg (&msg) < 0)
            trace_printf ("can-rx: error sending console data response\n");
    }
    return;
error:
    send_nak (request);
}

/* serial -> can */
static void canservices_console_task (void *arg __attribute((unused)))
{
    struct canmsg msg;
    int seq = 0;

    for (;;) {
        msg.dlen = serial_recv (msg.data, sizeof (msg.data), 1);

        if (msg.dlen > 0) {
            msg.pri = 1;
            msg.dst = console.dstaddr;
            msg.src = srcaddr;
            msg.type = CANMSG_TYPE_DAT;
            msg.object = console.object;
            msg.eot = serial_recv_available () == 0 || seq == 15;
            msg.seq = seq++;

            if (send_msg (&msg) < 0) {
                trace_printf ("can-console: error sending console data\n");
                continue;
            }
            if (msg.eot) { // suspend task pending ACK
                ulTaskNotifyTake (pdTRUE, pdMS_TO_TICKS (500));
                seq = 0;
            }
        }
    }
}

static void canservices_rx_task (void *arg __attribute((unused)))
{
    struct canmsg msg;

    for (;;) {
        if (canbus_recv (&msg, -1) == 0) {
            trace_msg (&msg);

            switch (msg.type) {
                case CANMSG_TYPE_WO:
                case CANMSG_TYPE_RO:
                case CANMSG_TYPE_WNA:
                case CANMSG_TYPE_DAT:
                    switch (msg.object) {
                        case CANMSG_OBJ_ECHO:
                            canservices_echo (&msg);
                            break;
                        case CANMSG_OBJ_POWER:
                            canservices_power (&msg);
                            break;
                        case CANMSG_OBJ_CONSOLECONN:
                            canservices_consoleconn (&msg);
                            break;
                        case CANMSG_OBJ_CONSOLEDISC:
                            canservices_consoledisc (&msg);
                            break;
                        case CANMSG_OBJ_CONSOLERECV:
                            canservices_consolerecv (&msg);
                            break;
                        case CANMSG_OBJ_SHUTDOWN:
                            canservices_shutdown (&msg);
                            break;
                        default: // unknown object id
                            send_nak (&msg);
                            break;
                    }
                    break;
                case CANMSG_TYPE_ACK:
                    if (console.connected && msg.object == console.object)
                        xTaskNotifyGive (console.task);
                    break;
                case CANMSG_TYPE_NAK:
                case CANMSG_TYPE_SIG:
                    break;
            }
        }
    }
}

void canservices_init (void)
{
    srcaddr = address_get () | CANMSG_ADDR_CONTROL;

    canbus_filter_set (0, (srcaddr << CANMSG_DST_SHIFT), CANMSG_DST_MASK);

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
