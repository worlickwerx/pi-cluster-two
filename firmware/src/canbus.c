/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/f1/nvic.h>
#include <libopencm3/stm32/can.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "canbus.h"
#include "trace.h"

static const uint32_t baudrate = 125000;

static QueueHandle_t canrxq;

struct canmsg {
        uint32_t        msgid;          // Message ID
        uint32_t        fmi;            // Filter index
        uint8_t         length;         // Data length
        uint8_t         data[8];        // Received data
        uint8_t         xmsgidf:1;      // Extended message flag
        uint8_t         rtrf:1;         // RTR flag
        uint8_t         fifo:1;         // RX Fifo 0 or 1
};

static void canbus_rx_task (void *arg __attribute((unused)))
{
    struct canmsg msg;

    for (;;) {
        if (xQueueReceive (canrxq, &msg, portMAX_DELAY) == pdPASS) {
            // FIXME: handle 'msg'
        }
    }
}

/*
static void canbus_xmit (uint32_t id,
                         bool ext,
                         bool rtr,
                         uint8_t length,
                         void *data)
{
    while (can_transmit (CAN1, id, ext, rtr, length, (uint8_t*)data) == -1)
        taskYIELD();
}
*/

static void canbus_rx_isr (uint8_t fifo, unsigned msgcount)
{
    struct canmsg msg;
    bool xmsgidf, rtrf;

    while (msgcount-- > 0) {
        can_receive(CAN1,
                    fifo,
                    true,                   // Release
                    &msg.msgid,
                    &xmsgidf,               // true if msgid is extended
                    &rtrf,                  // true if requested transmission
                    (uint8_t *)&msg.fmi,    // Matched filter index
                    &msg.length,            // Returned length
                    msg.data,
                    NULL);                  // Unused timestamp
        msg.xmsgidf = xmsgidf;
        msg.rtrf = rtrf;
        msg.fifo = fifo;
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

    rcc_periph_clock_enable (RCC_AFIO);
    rcc_peripheral_enable_clock (&RCC_APB1ENR, RCC_APB1ENR_CAN1EN);
    rcc_periph_clock_enable (RCC_GPIOB);

    gpio_set_mode (GPIOB,
                   GPIO_MODE_OUTPUT_50_MHZ,
                   GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                   GPIO_CAN_PB_TX);

    gpio_set_mode(GPIOB,
                  GPIO_MODE_INPUT,
                  GPIO_CNF_INPUT_FLOAT,
                  GPIO_CAN_PB_RX);

    // map CAN1 CAN_RX=PB8, CAN_TX=PB9
    gpio_primary_remap (AFIO_MAPR_SWJ_CFG_FULL_SWJ,
                        AFIO_MAPR_CAN1_REMAP_PORTB);

    can_reset (CAN1);

    if (!(baud = lookup_baud (baudrate)))
        FATAL ("unknown baud rate");

    can_init (CAN1,
              false,            // ttcm=off
              false,            // disable auto bus off
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

    // FIXME set up filters for this node's CAN address and bcast

    can_filter_id_mask_16bit_init (0,                // Filter bank 0
                                   0x000 << 5,
                                   0x001 << 5,       // LSB == 0
                                   0x000 << 5,
                                   0x001 << 5,       // Not used
                                   0,                // FIFO 0
                                   true);

    can_filter_id_mask_16bit_init (1,                // Filter bank 1
                                   0x010 << 5,
                                   0x001 << 5,       // LSB == 1 (no match)
                                   0x001 << 5,
                                   0x001 << 5,       // Match when odd
                                   1,                // FIFO 1
                                   true);

    canrxq = xQueueCreate (33, sizeof (struct canmsg));

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
