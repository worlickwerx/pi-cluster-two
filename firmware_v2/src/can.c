#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>
#include <string.h>

#include "canmgr_proto.h"
#include "debug.h"
#include "activity.h"
#include "can.h"

static uint8_t addr_mod;
static uint8_t addr_node;

static CAN_HandleTypeDef    can1;
static CanTxMsgTypeDef      tx_msg;
static CanRxMsgTypeDef      rx_msg;

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
    return 0;
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

    // 125Kbaud
    can1.Init.SJW = CAN_SJW_1TQ;
    can1.Init.BS1 = CAN_BS1_8TQ;
    can1.Init.BS2 = CAN_BS2_3TQ;
    can1.Init.Prescaler = 24;

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
        (void)can_recv (&fr, 0);
    }
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
