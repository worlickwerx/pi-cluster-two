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
#define SERIAL_RX_QUEUE_TRIGGER 8

static StreamBufferHandle_t serialrxq;

/* Interrupt handler puts characters into queue.
 */
void usart1_isr (void)
{
    unsigned char c;
    BaseType_t woke = pdFALSE;

    while (USART_SR (USART1) & USART_SR_RXNE) {
        c = USART_DR (USART1);
        xStreamBufferSendFromISR (serialrxq, &c, 1, &woke);
    }
}

int serial_recv (unsigned char *buf, int bufsize, int timeout)
{
    int n;

    while ((n = xStreamBufferReceive (serialrxq, buf, bufsize, timeout)) == 0)
        taskYIELD ();
    return n;
}

/* This function blocks while the transmit queue is busy.
 */
void serial_send (const unsigned char *buf, int len)
{
    int count;

    for (count = 0; count < len; count++) {
        while ((USART_SR (USART1) & USART_SR_TXE) == 0)
            taskYIELD ();
        usart_send_blocking (USART1, buf[count]);
    }
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

    serialrxq = xStreamBufferCreate (SERIAL_RX_QUEUE_DEPTH,
                                     SERIAL_RX_QUEUE_TRIGGER);

    nvic_enable_irq (NVIC_USART1_IRQ);
    usart_enable (USART1);
    usart_enable_rx_interrupt (USART1);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
