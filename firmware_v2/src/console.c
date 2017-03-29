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
    uart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_DeInit (&uart1) != HAL_OK)
        FATAL ("HAL_UART_DeInint failed");
    if (HAL_UART_Init (&uart1) != HAL_OK)
        FATAL ("HAL_UART_Init failed");

    head = tail = tail_hist = tail_cursor = 0;

    itm_printf ("Console: initialized\n");
}

void console_finalize (void)
{
}

// chars arriving at 115kb (115 per ms), bursty, and buffered;
// chars departing via CAN at up to 8 per update cycle.
void console_update (void)
{
    if (uart1.State == HAL_UART_STATE_READY && uart1.Lock == HAL_UNLOCKED) {
        uint32_t t0 = HAL_GetTick ();

        uart1.Lock = HAL_LOCKED;
        uart1.ErrorCode = HAL_UART_ERROR_NONE;
        uart1.State = HAL_UART_STATE_BUSY_RX;
        while (__HAL_UART_GET_FLAG (&uart1, UART_FLAG_RXNE) != RESET) {
            recv_buf[postincr (&head)] = uart1.Instance->DR & 0xff;
            if (head == tail_hist)
                postincr (&tail_hist);
            if (head == tail_cursor)
                postincr (&tail_cursor);
            if (head == tail) {
                postincr (&tail);
                break;
            }
            if (timesince (t0) > 10)
                break;
        }
        uart1.State = HAL_UART_STATE_READY;
        uart1.Lock = HAL_UNLOCKED;
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
