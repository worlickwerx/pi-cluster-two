#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

#include "debug.h"
#include "console.h"
#include "util.h"

const uint16_t uart1_tx_pin = GPIO_PIN_9;
GPIO_TypeDef *uart1_tx_port = GPIOA;

const uint16_t uart1_rx_pin = GPIO_PIN_10;
GPIO_TypeDef *uart1_rx_port = GPIOA;

UART_HandleTypeDef uart1;

static uint8_t rx_buf[8192];
static unsigned int rx_head, rx_tail, rx_tail_hist, rx_tail_cursor;

static uint8_t tx_buf[1024];
static unsigned int tx_head, tx_tail;

static unsigned int postincr (unsigned int *i)
{
    unsigned int last = *i;
    if (++(*i) == sizeof (rx_buf))
        *i = 0;
    return last;
}

/* This presumes 9B is not enabled.
 * There is no interlock between TX and RX unlike HAL (see how that goes!)
 * It overrides weak function in HAL startup assembly.
 */
void USART1_IRQHandler (void)
{
    /* parity error */
    if (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_PE) != RESET
        && __HAL_UART_GET_IT_SOURCE (&uart1, UART_IT_PE) != RESET) {
        __HAL_UART_CLEAR_PEFLAG (&uart1);
    }
    /* framing error */
    if (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_FE) != RESET
        && __HAL_UART_GET_IT_SOURCE (&uart1, UART_IT_ERR) != RESET) {
        __HAL_UART_CLEAR_PEFLAG (&uart1);
    }
    /* noise error */
    if (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_NE) != RESET
        && __HAL_UART_GET_IT_SOURCE (&uart1, UART_IT_ERR) != RESET) {
        __HAL_UART_CLEAR_PEFLAG (&uart1);
    }
    /* over-run error */
    if (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_ORE) != RESET
        && __HAL_UART_GET_IT_SOURCE (&uart1, UART_IT_ERR) != RESET) {
        __HAL_UART_CLEAR_PEFLAG (&uart1);
    }
    /* receive buffer not empty */
    if (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_RXNE) != RESET
        && __HAL_UART_GET_IT_SOURCE (&uart1, UART_IT_RXNE) != RESET) {
        rx_buf[postincr (&rx_head)] = uart1.Instance->DR & 0xff;
        if (rx_head == rx_tail)
            postincr (&rx_tail);
        if (rx_head == rx_tail_hist)
            postincr (&rx_tail_hist);
        if (rx_head == rx_tail_cursor)
            postincr (&rx_tail_cursor);
    }
    /* transmit buffer empty */
    if (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_TXE) != RESET
        && __HAL_UART_GET_IT_SOURCE (&uart1, UART_IT_TXE) != RESET) {
        if (tx_tail != tx_head) {
            uart1.Instance->DR = tx_buf[postincr (&tx_tail)];
            __HAL_UART_DISABLE_IT (&uart1, UART_IT_TXE);
            __HAL_UART_ENABLE_IT (&uart1, UART_IT_TC);
        } else {
            __HAL_UART_DISABLE_IT (&uart1, UART_IT_TXE);
        }
    }
    /* transmit complete */
    if (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_TC) != RESET
        && __HAL_UART_GET_IT_SOURCE (&uart1, UART_IT_TC) != RESET) {
        if (tx_tail != tx_head)
            __HAL_UART_ENABLE_IT (&uart1, UART_IT_TXE);
        __HAL_UART_DISABLE_IT (&uart1, UART_IT_TC);
    }
}

void console_setup (void)
{
    GPIO_InitTypeDef g;

    __HAL_RCC_USART1_CLK_ENABLE ();
    __HAL_RCC_GPIOA_CLK_ENABLE ();
    __HAL_RCC_AFIO_CLK_ENABLE ();
    __HAL_AFIO_REMAP_USART1_DISABLE ();

    /* TX */
    g.Mode = GPIO_MODE_AF_PP;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Pin = uart1_tx_pin;
    HAL_GPIO_Init (uart1_tx_port, &g);

    /* RX */
    g.Mode = GPIO_MODE_AF_INPUT;
    g.Pull = GPIO_PULLUP;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    g.Pin = uart1_rx_pin;
    HAL_GPIO_Init (uart1_rx_port, &g);

    HAL_NVIC_SetPriority (USART1_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ (USART1_IRQn);

    uart1.Instance = USART1;
    uart1.Init.BaudRate = 115200;
    uart1.Init.WordLength = UART_WORDLENGTH_8B;
    uart1.Init.StopBits = UART_STOPBITS_1;
    uart1.Init.Parity = UART_PARITY_NONE;
    uart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart1.Init.Mode = UART_MODE_TX_RX;
    uart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_DeInit (&uart1) != HAL_OK)
        FATAL ("HAL_UART_DeInint failed");
    if (HAL_UART_Init (&uart1) != HAL_OK)
        FATAL ("HAL_UART_Init failed");

    rx_head = rx_tail = rx_tail_hist = rx_tail_cursor = 0;
    tx_head = tx_tail = 0;

    __HAL_UART_ENABLE_IT (&uart1, UART_IT_PE);
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_ERR);
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);

    __HAL_UART_DISABLE_IT (&uart1, UART_IT_TXE);
    __HAL_UART_DISABLE_IT (&uart1, UART_IT_TC);

    itm_printf ("Console: initialized\n");
}

void console_finalize (void)
{
}

void console_update (void)
{
}

void console_send (uint8_t *buf, int len)
{
    int i;

    __HAL_UART_DISABLE_IT (&uart1, UART_IT_TXE);
    for (i = 0; i < len; i++) {
        tx_buf[postincr (&tx_head)] = buf[i];
        if (tx_head == tx_tail)
            postincr (&tx_tail);
    }
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_TXE);
}

int console_available (void)
{
    return rx_head != rx_tail;
}

void console_reset (void)
{
    __HAL_UART_DISABLE_IT (&uart1, UART_IT_RXNE);
    rx_tail = rx_head;
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);
}

int console_recv (uint8_t *buf, int len)
{
    int i = 0;
    __HAL_UART_DISABLE_IT (&uart1, UART_IT_RXNE);
    while (i < len && rx_tail != rx_head)
        buf[i++] = rx_buf[postincr (&rx_tail)];
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);
    return i;
}

int console_history_next (uint8_t *buf, int len)
{
    int i = 0;
    __HAL_UART_DISABLE_IT (&uart1, UART_IT_RXNE);
    while (i < len && rx_tail_cursor != rx_head)
        buf[i++] = rx_buf[postincr (&rx_tail_cursor)];
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);
    return i;
}

void console_history_reset (void)
{
    __HAL_UART_DISABLE_IT (&uart1, UART_IT_RXNE);
    rx_tail_cursor = rx_tail_hist;
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
