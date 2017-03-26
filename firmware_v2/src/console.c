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

static uint8_t recv_buf[1024];
static unsigned int head, tail, tail_hist, tail_cursor;

static unsigned int postincr (unsigned int *i)
{
    unsigned int last = *i;
    if (++(*i) == sizeof (recv_buf))
        *i = 0;
    return last;
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

    uart1.Instance = USART1;
    uart1.Init.BaudRate = 115200;
    uart1.Init.WordLength = UART_WORDLENGTH_8B;
    uart1.Init.StopBits = UART_STOPBITS_1;
    uart1.Init.Parity = UART_PARITY_NONE;
    uart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uart1.Init.Mode = UART_MODE_TX_RX;
    //uart1.Init.OverSampling = 16; // needed?

    if (HAL_UART_Init (&uart1) != HAL_OK)
        FATAL ("HAL_UART_Init failed");

    head = tail = tail_hist = tail_cursor = 0;

    itm_printf ("Console: initialized\n");
}

void console_finalize (void)
{
}

void console_update (void)
{
    HAL_StatusTypeDef res;
    uint8_t c;

    if ((res = HAL_UART_Receive (&uart1, &c, 1, 0)) == HAL_OK) {
        recv_buf[postincr (&head)] = c;
        if (head == tail)
            postincr (&tail);
        if (head == tail_hist)
            postincr (&tail_hist);
        if (head == tail_cursor)
            postincr (&tail_cursor);
    } else if (res != HAL_TIMEOUT) {
        itm_printf ("Console: receive error %d\n", res);
    }
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
    tail = head;
}

int console_recv (uint8_t *buf, int len)
{
    int i = 0;
    while (i < len && tail != head)
        buf[i++] = recv_buf[postincr (&tail)];
    return i;
}

int console_history_next (uint8_t *buf, int len)
{
    int i = 0;
    while (i < len && tail_cursor != head)
        buf[i++] = recv_buf[postincr (&tail_cursor)];
    return i;
}

void console_history_reset (void)
{
    tail_cursor = tail_hist;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
