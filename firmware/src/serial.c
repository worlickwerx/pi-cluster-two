/* SPDX-License-Identifier: GPL-3.0-or-later */

/* serial.c - USART 1 connected to pi console
 *
 * PA9 - TX (connected to pi GPIO15/RXD)
 * PA10 - RX (connected to pi GPIO14/TXD)
 *
 * 115200,8n1 and no flow control
 */

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stream_buffer.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/f1/nvic.h>

#include "trace.h"
#include "serial.h"

/* STM32F103C8T6 total RAM = 20K */

#define SERIAL_RX_QUEUE_DEPTH 8192
#define SERIAL_TX_QUEUE_DEPTH 128

#define RX_OVERRUN_RESUME   256

static int serialrxq_overrun;

static StreamBufferHandle_t serialrxq;
static StreamBufferHandle_t serialtxq;

/* Interrupt handler transfers characters to/from queues.
 */
void usart1_isr (void)
{
    unsigned char c;
    BaseType_t woke = pdFALSE;

    if (((USART_CR1(USART1) & USART_CR1_RXNEIE) != 0) &&
        ((USART_SR(USART1) & USART_SR_RXNE) != 0)) { // rx buf not empty
        c = usart_recv (USART1);
        if (serialrxq_overrun > 0)
            serialrxq_overrun++;
        else {
            if (xStreamBufferSendFromISR (serialrxq, &c, 1, &woke) != 1)
                serialrxq_overrun++;
        }
    }
    if (((USART_CR1(USART1) & USART_CR1_TXEIE) != 0) &&
        ((USART_SR(USART1) & USART_SR_TXE) != 0)) { // tx buf empty

        if (xStreamBufferReceiveFromISR (serialtxq, &c, 1, &woke) == 1)
            usart_send (USART1, c);
        else
            usart_disable_tx_interrupt (USART1);
    }
    portYIELD_FROM_ISR (woke);
}

int serial_recv (unsigned char *buf, int bufsize, int timeout)
{
    int ticks = timeout >= 0 ? pdMS_TO_TICKS (timeout) : portMAX_DELAY;
    int n;

    xStreamBufferSetTriggerLevel (serialrxq, bufsize);
    n = xStreamBufferReceive (serialrxq, buf, bufsize, ticks);

    if (n > 0 && serialrxq_overrun > 0) {
        if (xStreamBufferSpacesAvailable(serialrxq) >= RX_OVERRUN_RESUME) {
            serialrxq_overrun = 0;
        }
    }
    return n;
}

int serial_send (const unsigned char *buf, int len, int timeout)
{
    int ticks = timeout >= 0 ? pdMS_TO_TICKS (timeout) : portMAX_DELAY;
    int n;

    n = xStreamBufferSend (serialtxq, buf, len, ticks);
    usart_enable_tx_interrupt (USART1);

    return n;
}

void serial_rx_enable (void)
{
    usart_enable_rx_interrupt (USART1);
}

void serial_rx_disable (void)
{
    usart_disable_rx_interrupt (USART1);
}

void serial_init (void)
{
    rcc_periph_clock_enable (RCC_GPIOA);
    rcc_periph_clock_enable (RCC_USART1);

    // PA9 == GPIO_USART1_TX
    gpio_set_mode (GPIOA,
                   GPIO_MODE_OUTPUT_50_MHZ,
                   GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                   GPIO_USART1_TX);

    // PA10 == GPIO_USART1_RX
    gpio_set_mode (GPIOA,
                   GPIO_MODE_INPUT,
                   GPIO_CNF_INPUT_FLOAT,
                   GPIO_USART1_RX);

    usart_set_baudrate (USART1, 115200);

    usart_set_databits (USART1, 8);
    usart_set_parity (USART1, USART_PARITY_NONE);
    usart_set_stopbits (USART1, USART_STOPBITS_1);

    usart_set_mode(USART1, USART_MODE_TX_RX);
    usart_set_flow_control (USART1, USART_FLOWCONTROL_NONE);

    serialrxq = xStreamBufferCreate (SERIAL_RX_QUEUE_DEPTH, 1);
    serialtxq = xStreamBufferCreate (SERIAL_TX_QUEUE_DEPTH, 1);

    nvic_set_priority (NVIC_USART1_IRQ, 12<<4);
    nvic_enable_irq (NVIC_USART1_IRQ);
    usart_enable (USART1);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
