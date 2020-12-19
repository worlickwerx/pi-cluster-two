/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f1/nvic.h>
#include <libopencm3/stm32/can.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "string.h"

#include "canbus.h"
#include "matrix.h"
#include "trace.h"
#include "address.h"
#include "canmsg.h"
#include "canmsg_v1.h"
#include "power.h"

static const uint32_t baudrate = 125000;

static QueueHandle_t canrxq;

/* Assume native little endian (ARM can be either, but usually this).
 * Network byte order is big endian.
 */
static uint16_t htons (uint16_t val)
{
    return (val << 8) | (val >> 8);
}

static void canbus_xmit (uint32_t id,
                         bool ext,
                         bool rtr,
                         uint8_t length,
                         void *data)
{
    matrix_pulse_green (); // blink the activity LED

    while (can_transmit (CAN1, id, ext, rtr, length, (uint8_t*)data) == -1)
        taskYIELD();
}

static int canbus_xmit_v1 (const struct canmsg_v1 *msg)
{
    struct canmsg_raw raw;

    if (canmsg_v1_encode (msg, &raw) < 0)
        return -1;
    canmsg_v1_trace (msg);
    canbus_xmit (raw.msgid, raw.xmsgidf, 0, raw.length, raw.data);
    return 0;
}

static void canbus_v1_nak (const struct canmsg_v1 *request)
{
    struct canmsg_v1 msg = *request;

    if (request->type == CANMSG_V1_TYPE_WNA)
        return;

    msg.type = CANMSG_V1_TYPE_NAK;
    msg.dst = msg.src;
    msg.src = msg.node = address_get ();
    msg.dlen = 0;

    if (canbus_xmit_v1 (&msg) < 0)
        trace_printf ("canbus-rx: error encoding NAK response\n");
}

static void canbus_v1_echo (const struct canmsg_v1 *request)
{
    struct canmsg_v1 msg = *request;

    if (request->type != CANMSG_V1_TYPE_WO) {
        canbus_v1_nak (request);
        return;
    }

    msg.type = CANMSG_V1_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = msg.node = address_get ();

    if (canbus_xmit_v1 (&msg) < 0)
        trace_printf ("canbus-rx: error encoding echo response\n");
}

static void canbus_v1_power (const struct canmsg_v1 *request)
{
    struct canmsg_v1 msg = *request;

    if (request->type != CANMSG_V1_TYPE_WO)
        goto error;
    if (request->dlen != 1)
        goto error;
    if (request->data[0] == 0)
        power_set_state (false);
    else if (request->data[0] == 1)
        power_set_state (true);
    else
        goto error;

    msg.type = CANMSG_V1_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = msg.node = address_get ();

    if (canbus_xmit_v1 (&msg) < 0)
        trace_printf ("canbus-rx: error encoding power response\n");
    return;
error:
    canbus_v1_nak (request);
}

static void canbus_v1_power_measure (const struct canmsg_v1 *request)
{
    struct canmsg_v1 msg = *request;
    uint16_t ma;

    if (request->type != CANMSG_V1_TYPE_RO)
        goto error;
    if (request->dlen != 0)
        goto error;

    msg.type = CANMSG_V1_TYPE_ACK;
    msg.dst = msg.src;
    msg.src = msg.node = address_get ();

    power_get_measurements (&ma, NULL);
    msg.dlen = 2;
    *(uint16_t *)msg.data = htons (ma);

    if (canbus_xmit_v1 (&msg) < 0)
        trace_printf ("canbus-rx: error encoding power measure response\n");
    return;
error:
    canbus_v1_nak (request);
}

static void canbus_rx_task (void *arg __attribute((unused)))
{
    struct canmsg_raw raw;
    struct canmsg_v1 msg;

    for (;;) {
        if (xQueueReceive (canrxq, &raw, portMAX_DELAY) == pdPASS) {

            matrix_pulse_green (); // blink the activity LED

            if (canmsg_v1_decode (&raw, &msg) < 0) {
                trace_printf ("canbus-rx: error decoding received message\n");
                continue;
            }

            canmsg_v1_trace (&msg);

            switch (msg.type) {
                case CANMSG_V1_TYPE_WO:
                case CANMSG_V1_TYPE_RO:
                case CANMSG_V1_TYPE_WNA:
                    switch (msg.object) {
                        case CANMSG_V1_OBJ_ECHO:
                            canbus_v1_echo (&msg);
                            break;
                        case CANMSG_V1_OBJ_POWER:
                            canbus_v1_power (&msg);
                            break;
                        case CANMSG_V1_OBJ_POWER_MEASURE:
                            canbus_v1_power_measure (&msg);
                            break;
                        default: // unknown object id
                            canbus_v1_nak (&msg);
                            break;
                    }
                    break;
                case CANMSG_V1_TYPE_DAT:
                case CANMSG_V1_TYPE_ACK:
                case CANMSG_V1_TYPE_NAK:
                case CANMSG_V1_TYPE_SIG:
                    break;
            }
        }
    }
}

static void canbus_rx_isr (uint8_t fifo, unsigned msgcount)
{
    struct canmsg_raw msg;
    uint8_t fmi;

    while (msgcount-- > 0) {
        can_receive(CAN1,
                    fifo,
                    true,                   // Release
                    &msg.msgid,
                    &msg.xmsgidf,           // true if msgid is extended
                    &msg.rtrf,              // true if requested transmission
                    &fmi,                   // Matched filter index
                    &msg.length,            // Returned length
                    msg.data,
                    NULL);                  // Unused timestamp

        // If the queue is full, the message is lost
        xQueueSendToBackFromISR (canrxq, &msg, NULL);
    }
}

// ISR for CAN FIFO 0 - redirect to common handler
void usb_lp_can_rx0_isr (void)
{
    canbus_rx_isr (0, CAN_RF0R (CAN1) & 3);
}

// ISR for CAN FIFO 1 - redirect to common handler
void can_rx1_isr (void)
{
    canbus_rx_isr (1, CAN_RF1R (CAN1) & 3);
}

struct canbaud {
    uint32_t baud;
    uint32_t sjw;
    uint32_t ts1;
    uint32_t ts2;
    uint32_t prescaler;
};

// table generated using http://www.bittiming.can-wiki.info/
// assumes prescaler is clocked at 36 MHz (APB1/PCLK1)
static const struct canbaud btab[] = {
    {   10000, CAN_BTR_SJW_1TQ, CAN_BTR_TS1_13TQ, CAN_BTR_TS2_2TQ, 225 },
    {   50000, CAN_BTR_SJW_1TQ, CAN_BTR_TS1_13TQ, CAN_BTR_TS2_2TQ, 45 },
    {   83333, CAN_BTR_SJW_1TQ, CAN_BTR_TS1_13TQ, CAN_BTR_TS2_2TQ, 27 },
    {  125000, CAN_BTR_SJW_1TQ, CAN_BTR_TS1_13TQ, CAN_BTR_TS2_2TQ, 18 },
    {  250000, CAN_BTR_SJW_1TQ, CAN_BTR_TS1_13TQ, CAN_BTR_TS2_2TQ, 9 },
    {  500000, CAN_BTR_SJW_1TQ, CAN_BTR_TS1_10TQ, CAN_BTR_TS1_1TQ, 6 },
    {  800000, CAN_BTR_SJW_1TQ, CAN_BTR_TS1_12TQ, CAN_BTR_TS1_2TQ, 3 },
    { 1000000, CAN_BTR_SJW_1TQ, CAN_BTR_TS1_10TQ, CAN_BTR_TS1_1TQ, 3 },
};

static const struct canbaud *lookup_baud (uint32_t baud)
{
    unsigned int i;
    for (i = 0; i < sizeof (btab) / sizeof (btab[0]); i++)
        if (btab[i].baud == baud)
            return &btab[i];
    return NULL;
}

void canbus_init (void)
{
    const struct canbaud *baud;
    uint8_t addr = address_get ();

    rcc_periph_clock_enable (RCC_AFIO);
    rcc_peripheral_enable_clock (&RCC_APB1ENR, RCC_APB1ENR_CAN1EN);
    rcc_periph_clock_enable (RCC_GPIOB);

    gpio_set_mode (GPIOB,
                   GPIO_MODE_OUTPUT_50_MHZ,
                   GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                   GPIO_CAN_PB_TX); // PB9

    gpio_set_mode(GPIOB,
                  GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_FLOAT,
                  GPIO_CAN_PB_RX); // PB8

    gpio_primary_remap (AFIO_MAPR_SWJ_CFG_FULL_SWJ,
                        AFIO_MAPR_CAN1_REMAP_PORTB);

    can_reset (CAN1);

    if (!(baud = lookup_baud (baudrate)))
        FATAL ("unknown baud rate");

    can_init (CAN1,
              false,            // ttcm=off
              true,             // enable auto bus off management
              true,             // auto wakeup mode
              false,            // enable auto retransmission
              true,             // receive FIFO locked mode
              false,            // transmit FIFO priority (msg id)
              baud->sjw,        // baud rate params
              baud->ts1,
              baud->ts2,
              baud->prescaler,
              false,            // disable loopback mode
              false);           // disable silent mode

    /* locally addressed - route to FIFO 0 */
    can_filter_id_mask_32bit_init (0,                 // filter bank 0
                                   (addr << CANMSG_V1_DST_SHIFT) << 3, // id
                                   CANMSG_V1_DST_MASK << 3, // mask
                                   0,                 // FIFO 0
                                   true);             // enable

    /* (disabled) promiscuous - route to FIFO 1 */
    can_filter_id_mask_32bit_init (1, 0, 0, 1, false);

    canrxq = xQueueCreate (33, sizeof (struct canmsg_raw));

    nvic_enable_irq (NVIC_USB_LP_CAN_RX0_IRQ);
    nvic_enable_irq (NVIC_CAN_RX1_IRQ);

    can_enable_irq (CAN1, CAN_IER_FMPIE0 | CAN_IER_FMPIE1);

    xTaskCreate (canbus_rx_task,
                 "canrx",
                 400,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 NULL);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
