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

static const uint32_t baudrate = 125000;

static QueueHandle_t canrxq;

struct canmsg_raw {
        uint32_t        msgid;          // Message ID
        uint32_t        fmi;            // Filter index
        uint8_t         length;         // Data length
        uint8_t         data[8];        // Received data
        uint8_t         xmsgidf:1;      // Extended message flag
        uint8_t         rtrf:1;         // RTR flag
        uint8_t         fifo:1;         // RX Fifo 0 or 1
};

struct canmsg_v1 {
    /* 29 bit id */
    uint16_t pri:1; // 0=high, 1=low
    uint16_t dst:5;
    uint16_t src:5;
    uint32_t xpri:1; // 0=high, 1=low
    uint32_t type:3;
    uint32_t module:6;
    uint32_t node:6;
    uint32_t object:8; // only 2 bits in id; if >= uses data[0]
    /* 8 byte payload */
    uint8_t dlen;
    uint8_t data[8];
};

#define CANMSG_V1_DST_SHIFT (23)
#define CANMSG_V1_DST_MASK (0x1f << CANMSG_V1_DST_SHIFT)

#define CANMSG_V1_LEADER_ADDR (0x1d)
#define CANMSG_V1_BCAST_ADDR (0x1e)
#define CANMSG_V1_NOROUTE_ADDR (0x1f)

enum {
    CANMSG_V1_TYPE_RO = 0,
    CANMSG_V1_TYPE_WO = 1,
    CANMSG_V1_TYPE_WNA = 2,
    CANMSG_V1_TYPE_DAT = 3,
    CANMSG_V1_TYPE_ACK = 4,
    CANMSG_V1_TYPE_NAK = 6,
    CANMSG_V1_TYPE_SIG = 7,
};

enum {
    /* these ids are represented in 2 bit object id,
     * leaving the full data[0:7] for payload
     */
    CANMSG_V1_OBJ_HEARTBEAT     = 0,
    CANMSG_V1_OBJ_CONSOLERECV   = 1,
    CANMSG_V1_OBJ_CONSOLESEND   = 2,

    /* these ids are represented in 2 bits object id + 6 bits data[0],
     * leaving data[1:7] for payload
     */
    CANMSG_V1_OBJ_LED_IDENTIFY  = 3,
    CANMSG_V1_OBJ_POWER         = 4,
    CANMSG_V1_OBJ_ECHO          = 5,
    CANMSG_V1_OBJ_RESET         = 6,
    CANMSG_V1_OBJ_CONSOLECONN   = 7,
    CANMSG_V1_OBJ_CONSOLEDISC   = 8,
    CANMSG_V1_OBJ_CONSOLERING   = 9,
    CANMSG_V1_OBJ_POWER_MEASURE = 10,
    /* 0x80 - 0xff reserved for dynamically allocated CONSOLESEND objects */
    CANMSG_V1_OBJ_CONSOLEBASE = 0x80,
};

struct strtab {
    int id;
    const char *name;
};

struct strtab typestr_v1[] = {
    { CANMSG_V1_TYPE_RO,    "RO" },
    { CANMSG_V1_TYPE_WO,    "WO" },
    { CANMSG_V1_TYPE_WNA,   "WNA" },
    { CANMSG_V1_TYPE_DAT,   "D" },
    { CANMSG_V1_TYPE_ACK,   "ACK" },
    { CANMSG_V1_TYPE_NAK,   "NAK" },
    { CANMSG_V1_TYPE_SIG,   "SIG" },
};

struct strtab objstr_v1[] = {
    { CANMSG_V1_OBJ_HEARTBEAT,      "HB" },
    { CANMSG_V1_OBJ_CONSOLECONN,    "CONSOLECONN" },
    { CANMSG_V1_OBJ_CONSOLEDISC,    "CONSOLEDISC" },
    { CANMSG_V1_OBJ_CONSOLESEND,    "CONSOLESEND" },
    { CANMSG_V1_OBJ_CONSOLERECV,    "CONSOLERECV" },
    { CANMSG_V1_OBJ_CONSOLERING,    "CONSOLERING" },
    { CANMSG_V1_OBJ_POWER,          "POWER" },
    { CANMSG_V1_OBJ_RESET,          "RESET" },
    { CANMSG_V1_OBJ_ECHO,           "ECHO" },
    { CANMSG_V1_OBJ_LED_IDENTIFY,   "IDENTIFY" },
};

static const char *strtab_lookup (int id, const struct strtab *tab, size_t size)
{
    uint8_t i;
    for (i = 0; i < size / sizeof (struct strtab); i++) {
        if (tab[i].id == id)
            return tab[i].name;
    }
    return "?";
}

static int canmsg_v1_decode (const struct canmsg_raw *raw,
                             struct canmsg_v1 *msg)
{
    if (!raw->xmsgidf)
        return -1;
    msg->object = raw->msgid & 3;
    msg->node = (raw->msgid>>2) & 0x3f;
    msg->module = (raw->msgid>>8) & 0x3f;
    msg->type = (raw->msgid>>14) & 7;
    msg->xpri = (raw->msgid>>17) & 1;
    msg->src = (raw->msgid>>18) & 0x1f;
    msg->dst = (raw->msgid>>23) & 0x1f;
    msg->pri = (raw->msgid>>28) & 1;
    if (msg->object == 3) { // extended object id
        if (raw->length == 0)
            return -1;
        msg->object += raw->data[0];
        msg->dlen = raw->length - 1;
        memcpy (msg->data, &raw->data[1], msg->dlen);
    } else {
        msg->dlen = raw->length;
        memcpy (msg->data, raw->data, msg->dlen);
    }
    return 0;
}

static int canmsg_v1_encode (const struct canmsg_v1 *msg,
                             struct canmsg_raw *raw)
{
    int maxdata = msg->object >= 3 ? 7 : 8;

   if (msg->dlen > maxdata)
        return -1;
    raw->msgid = msg->object > 3 ? 3 : msg->object;
    raw->msgid |= msg->node<<2;
    raw->msgid |= msg->module<<8;
    raw->msgid |= msg->type<<14;
    raw->msgid |= msg->xpri<<17;
    raw->msgid |= msg->src<<18;
    raw->msgid |= msg->dst<<23;
    raw->msgid |= msg->pri<<28;
    if (msg->object >= 3) { // extended object id
        raw->data[0] = msg->object - 3;
        raw->length = msg->dlen + 1;
        memcpy (&raw->data[1], msg->data, msg->dlen);
    } else {
        raw->length = msg->dlen;
        memcpy (raw->data, msg->data, msg->dlen);
    }
    raw->fmi = 0;
    raw->xmsgidf = 1;
    raw->rtrf = 0;
    raw->fifo = 0;
    return 0;
}

static void canmsg_v1_trace (const struct canmsg_v1 *msg)
{
    trace_printf ("%x->%x %s %s [%d bytes]\n",
                  msg->src,
                  msg->dst,
                  strtab_lookup (msg->type, typestr_v1, sizeof (typestr_v1)),
                  strtab_lookup (msg->object, objstr_v1, sizeof (objstr_v1)),
                  msg->dlen);
}

static void canbus_rx_task (void *arg __attribute((unused)))
{
    struct canmsg_raw raw;
    struct canmsg_v1 msg;

    for (;;) {
        if (xQueueReceive (canrxq, &raw, portMAX_DELAY) == pdPASS) {

            matrix_pulse_green (); // blink the activity LED

            if (canmsg_v1_decode (&raw, &msg) == 0)
                canmsg_v1_trace (&msg);
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
    struct canmsg_raw msg;
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
