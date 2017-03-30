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

static uint8_t recv_buf[8192];
static unsigned int head, tail, tail_hist, tail_cursor;

static unsigned int postincr (unsigned int *i)
{
    unsigned int last = *i;
    if (++(*i) == sizeof (recv_buf))
        *i = 0;
    return last;
}

/* override weak symbol in HAL startup assembly */
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
        recv_buf[postincr (&head)] = uart1.Instance->DR & 0xff;
        if (head == tail_hist)
            postincr (&tail_hist);
        if (head == tail_cursor)
            postincr (&tail_cursor);
        if (head == tail)
            postincr (&tail);
    }
    /* transmit buffer empty */
    if (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_TXE) != RESET
        && __HAL_UART_GET_IT_SOURCE (&uart1, UART_IT_TXE) != RESET) {
    }
    /* transmit complete */
    if (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_TC) != RESET
        && __HAL_UART_GET_IT_SOURCE (&uart1, UART_IT_TC) != RESET) {
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

    head = tail = tail_hist = tail_cursor = 0;

    __HAL_UART_ENABLE_IT (&uart1, UART_IT_PE);
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_ERR);
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);

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
    HAL_StatusTypeDef res;

    if ((res = HAL_UART_Transmit (&uart1, buf, len, 100)) != HAL_OK)
        itm_printf ("Console: transmit error %d\n", res);
}

int console_available (void)
{
    return head != tail;
}

void console_reset (void)
{
    __HAL_UART_DISABLE_IT (&uart1, UART_IT_RXNE);
    tail = head;
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);
}

int console_recv (uint8_t *buf, int len)
{
    int i = 0;
    __HAL_UART_DISABLE_IT (&uart1, UART_IT_RXNE);
    while (i < len && tail != head)
        buf[i++] = recv_buf[postincr (&tail)];
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);
    return i;
}

int console_history_next (uint8_t *buf, int len)
{
    int i = 0;
    __HAL_UART_DISABLE_IT (&uart1, UART_IT_RXNE);
    while (i < len && tail_cursor != head)
        buf[i++] = recv_buf[postincr (&tail_cursor)];
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);
    return i;
}

void console_history_reset (void)
{
    __HAL_UART_DISABLE_IT (&uart1, UART_IT_RXNE);
    tail_cursor = tail_hist;
    __HAL_UART_ENABLE_IT (&uart1, UART_IT_RXNE);
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
