#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>
#include <string.h>

#include "canmgr_proto.h"
#include "canmgr_dump.h"
#include "debug.h"
#include "activity.h"
#include "identify.h"
#include "power.h"
#include "console.h"
#include "can.h"

static uint8_t addr_mod;
static uint8_t addr_node;

static CAN_HandleTypeDef    can1;
static CanTxMsgTypeDef      tx_msg;
static CanRxMsgTypeDef      rx_msg;

static struct canmgr_frame console_connected = {
    .module = CANMGR_ADDR_NOROUTE,
    .node = CANMGR_ADDR_NOROUTE,
};
static int console_lastsent_needack = 0;
static int console_ringdump = 0;

void canobj_target_consoleconn (struct canmgr_frame *fr);
void canobj_target_consoledisc (struct canmgr_frame *fr);
void canobj_target_consolering (struct canmgr_frame *fr);
void canobj_target_consolerecv (struct canmgr_frame *fr);
void canobj_target_consolesend (struct canmgr_frame *fr);
void canobj_led_identify (struct canmgr_frame *fr);
void canobj_target_power (struct canmgr_frame *fr);
void canobj_echo (struct canmgr_frame *fr);
void canobj_unknown (struct canmgr_frame *fr);

int can_available (void)
{
    return __HAL_CAN_MSG_PENDING (&can1, CAN_FIFO0);
}

int can_recv (struct canmgr_frame *fr, uint32_t timeout_ms)
{
    struct rawcan_frame raw;

    if (HAL_CAN_Receive (&can1, CAN_FIFO0, timeout_ms) != HAL_OK)
        return -1;
    raw.id = rx_msg.IDE == CAN_ID_EXT ? rx_msg.ExtId : rx_msg.StdId;
    raw.dlen = rx_msg.DLC;
    memcpy (&raw.data[0], rx_msg.Data, rx_msg.DLC);
    if (canmgr_decode (fr, &raw) < 0)
        return -1;
    activity_pulse ();
    if (itm_enabled ()) {
        char buf[80];
        canmgr_dump (fr, buf, sizeof (buf));
        itm_printf ("%s\n", buf);
    }
    return 0;
}

int can_send (struct canmgr_frame *fr, uint32_t timeout_ms)
{
    struct rawcan_frame raw;

    if (canmgr_encode (fr, &raw) < 0)
        return -1;
    tx_msg.StdId = 0;
    tx_msg.ExtId = raw.id;
    tx_msg.IDE = CAN_ID_EXT;
    tx_msg.RTR = 0;
    tx_msg.DLC = raw.dlen;
    memcpy (tx_msg.Data, &raw.data[0], raw.dlen);
    if (HAL_CAN_Transmit (&can1, timeout_ms) != HAL_OK)
        return -1;
    activity_pulse ();
    if (itm_enabled ()) {
        char buf[80];
        canmgr_dump (fr, buf, sizeof (buf));
        itm_printf ("%s\n", buf);
    }
    return 0;
}

void can_send_ack (struct canmgr_frame *fr, int type, void *data, int len)
{
    int tmpaddr = fr->src;
    fr->src = fr->dst;
    fr->dst = tmpaddr;
    fr->pri = 0; // high priority
    fr->type = type;

    if (len > canmgr_maxdata (fr->object))
        return;
    memcpy (fr->data, data, len);
    fr->dlen = len;

    can_send (fr, 100);
}

void can_heartbeat_send (uint8_t *data, uint8_t len)
{
   struct canmgr_frame fr;

    fr.pri = 1;
    fr.dst = CANMGR_MODULE_CTRL;
    fr.src = addr_node | 0x10;
    fr.type = CANMGR_TYPE_WNA;
    fr.node = CANMGR_MODULE_CTRL;
    fr.module = CANMGR_ADDR_NOROUTE;
    fr.object = CANOBJ_HEARTBEAT;
    memcpy (fr.data, data, len);
    fr.dlen = len; // payload empty for now
    can_send (&fr, 100);
}

int can_console_send_ready (void)
{
    if (CONSOLE_UNCONNECTED (&console_connected))
        return 0;
    if (console_lastsent_needack)
        return 0;
    return 1;
}

void can_console_send (uint8_t *buf, int len)
{
    if (!can_console_send_ready ())
        return;
    if (len > canmgr_maxdata (console_connected.object))
        return;
    console_connected.dlen = len;
    memcpy (console_connected.data, buf, len);
    if (can_send (&console_connected, 100) < 0)
        return;
    console_lastsent_needack = 1;
}

void can_setup (uint8_t mod, uint8_t node)
{
    GPIO_InitTypeDef g;
    CAN_FilterConfTypeDef f;

    addr_node = node;
    addr_mod = mod;

    __HAL_RCC_CAN1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_AFIO_REMAP_CAN1_1(); // CAN_RX = PA11, CAN_TX = PA12

    g.Pin = GPIO_PIN_11;
    g.Mode = GPIO_MODE_AF_INPUT;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Pull = GPIO_NOPULL;
    HAL_GPIO_Init (GPIOA, &g);

    g.Pin = GPIO_PIN_12;
    g.Mode = GPIO_MODE_AF_PP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Pull = GPIO_NOPULL;
    HAL_GPIO_Init (GPIOA, &g);

    can1.Instance = CAN1;

    can1.pTxMsg = &tx_msg;
    can1.pRxMsg = &rx_msg;

    // 125Kbaud (prescaler clocked at 36 MHz (APB1/PCLK1)
    // http://www.bittiming.can-wiki.info/
    can1.Init.SJW = CAN_SJW_1TQ;
    can1.Init.BS1 = CAN_BS1_13TQ;
    can1.Init.BS2 = CAN_BS2_2TQ;
    can1.Init.Prescaler = 18;

    can1.Init.TTCM = DISABLE; // time triggered comms mode
    can1.Init.ABOM = DISABLE; // auto bus-off management
    can1.Init.AWUM = DISABLE; // auto wakeup mode
    can1.Init.NART = DISABLE; // no auto retransmission
    can1.Init.RFLM = DISABLE; // recv FIFO locked mode
    can1.Init.TXFP = DISABLE; // xmit FIFO priority
    can1.Init.Mode = CAN_MODE_NORMAL; // not loopback

    if (HAL_CAN_Init (&can1) != HAL_OK)
        FATAL ("HAL_CAN_Init failed");

    uint32_t can_mask = CANMGR_DST_MASK;
    uint32_t can_id = (addr_node|0x10) << CANMGR_DST_SHIFT;

    f.FilterNumber = 0;
    f.FilterMode = CAN_FILTERMODE_IDMASK;
    f.FilterScale = CAN_FILTERSCALE_32BIT;
    f.FilterIdHigh = (can_id << 3) >> 16;
    f.FilterIdLow = (can_id << 3) & 0xFFFF;
    f.FilterMaskIdHigh = (can_mask << 3) >> 16;
    f.FilterMaskIdLow = (can_mask << 3) & 0xFFFF;
    f.FilterFIFOAssignment = CAN_FILTER_FIFO0;
    f.FilterActivation = ENABLE;
    f.BankNumber = 14;

    if (HAL_CAN_ConfigFilter (&can1, &f) != HAL_OK)
        FATAL ("HAL_CAN_ConfigFilter failed");

    itm_printf ("CAN: initialized\n");
}

void can_finalize (void)
{
}

void can_update (void)
{
    struct canmgr_frame fr;

    if (can_available ()) {
        if (can_recv (&fr, 0) == 0) {
            switch (fr.object) {
                case CANOBJ_ECHO:
                    canobj_echo (&fr);
                    break;
                case CANOBJ_TARGET_POWER:
                    canobj_target_power (&fr);
                    break;
                case CANOBJ_LED_IDENTIFY:
                    canobj_led_identify (&fr);
                    break;
               case CANOBJ_TARGET_CONSOLECONN:
                    canobj_target_consoleconn (&fr);
                    break;
                case CANOBJ_TARGET_CONSOLEDISC:
                    canobj_target_consoledisc (&fr);
                    break;
                case CANOBJ_TARGET_CONSOLERECV:
                    canobj_target_consolerecv (&fr);
                    break;
                case CANOBJ_TARGET_CONSOLERING:
                    canobj_target_consolering (&fr);
                    break;
                default:
                    if (fr.object == console_connected.object) {
                        canobj_target_consolesend (&fr);
                        break;
                    }
                    canobj_unknown (&fr);
                    break;
            }
        }
    }
    if (can_console_send_ready ()) {
        uint8_t buf[8];
        int max = canmgr_maxdata (console_connected.object);
        int len = 0;

        if (max > sizeof (buf))
            max = sizeof (buf);
        if (console_ringdump) {
            if ((len = console_history_next (buf, max)) == 0)
                console_ringdump = 0; // EOF
        } else {
            len = console_recv (buf, max);
        }
        if (len > 0) {
            can_console_send (buf, len);
        }
    }
}

/* can objects */

void canobj_led_identify (struct canmgr_frame *fr)
{
    uint8_t val;

    switch (fr->type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 1)
                goto nak;
            identify_set (fr->data[0]);
            can_send_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
            if (fr->dlen != 0)
                goto nak;
            identify_get (&val);
            can_send_ack (fr, CANMGR_TYPE_ACK, &val, 1);
            break;
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    can_send_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_power (struct canmgr_frame *fr)
{
    uint8_t val;

    switch (fr->type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 1)
                goto nak;
            power_set (fr->data[0]);
            can_send_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
            if (fr->dlen != 0)
                goto nak;
            power_get (&val);
            can_send_ack (fr, CANMGR_TYPE_ACK, &val, 1);
            break;
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    can_send_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_echo (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
        case CANMGR_TYPE_DAT:
            can_send_ack (fr, CANMGR_TYPE_ACK, fr->data, fr->dlen);
            break;
        case CANMGR_TYPE_ACK:
        case CANMGR_TYPE_NAK:
            break;
        default:
            break;
    }
}

void canobj_unknown (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
        case CANMGR_TYPE_DAT:
            can_send_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
            break;
        default:
            break;
    }
}

void canobj_target_consoleconn (struct canmgr_frame *fr)
{
    uint8_t val[3];

    switch (fr->type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 3)
                goto nak;
            /* Prepare reusable outgoing DAT packet for console output
             */
            console_connected.pri = 1;
            console_connected.dst = fr->data[1];
            console_connected.src = addr_node | 0x10;;
            console_connected.xpri = 1;
            console_connected.type = CANMGR_TYPE_DAT;
            console_connected.module = fr->data[0];
            console_connected.node = fr->data[1];
            console_connected.object = fr->data[2];
            console_reset (); // dump only new data
            can_send_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
            if (fr->dlen != 0)
                goto nak;
            val[0] = console_connected.module;
            val[1] = console_connected.node;
            val[2] = console_connected.object;
            can_send_ack (fr, CANMGR_TYPE_ACK, val, 3);
            break;
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    can_send_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_consoledisc (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 3 || console_connected.module != fr->data[0]
                              || console_connected.node != fr->data[1]
                              || console_connected.object != fr->data[2])
                goto nak;
            console_connected.node = CANMGR_ADDR_NOROUTE;
            console_connected.module = CANMGR_ADDR_NOROUTE;
            can_send_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            console_lastsent_needack = 0;
            break;
        case CANMGR_TYPE_RO:
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    can_send_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_consolering (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_WO:
            if (fr->dlen != 3 || console_connected.module != fr->data[0]
                              || console_connected.node != fr->data[1]
                              || console_connected.object != fr->data[2])
                goto nak;
            console_history_reset ();
            console_ringdump = 1;
            can_send_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_RO:
        case CANMGR_TYPE_DAT:
            goto nak;
        default:
            break;
    }
    return;
nak:
    can_send_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_consolerecv (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_DAT:
            console_send (fr->data, fr->dlen);
            can_send_ack (fr, CANMGR_TYPE_ACK, NULL, 0);
            break;
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
            goto nak;
        default:
            break;
    }
    return;
nak:
    can_send_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

void canobj_target_consolesend (struct canmgr_frame *fr)
{
    switch (fr->type) {
        case CANMGR_TYPE_WO:
        case CANMGR_TYPE_RO:
        case CANMGR_TYPE_DAT:
            goto nak;
        case CANMGR_TYPE_ACK:
        case CANMGR_TYPE_NAK:
            console_lastsent_needack = 0;
            break;
        default:
            break;
    }
    return;
nak:
    can_send_ack (fr, CANMGR_TYPE_NAK, NULL, 0);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
