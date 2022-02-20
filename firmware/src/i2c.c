/* SPDX-License-Identifier: GPL-3.0-or-later */

/* i2c.c - I2C2 (follower) connected to pi (leader)
 *
 * PB10 - SCL (connected to pi GPIO3)
 * PB11 - SDA (connected to pi GPIO2)
 *
 * Notes:
 * - Pi has on-board 1.8K pulls to 3V3 on its I2C bus lines
 * - pi-carrier v2 needs a mod to pull 4.7K pullups to STM32 3V3 not pi 3V3,
 *   otherwise i2c floats when pi is powered down
 * - See note in matrix.c about SMBA conflict on PB12 (worked around there).
 */

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/f1/nvic.h>

#include "trace.h"
#include "i2c.h"
#include "address.h"
#include "matrix.h"

#define FOLLOWER_ADDRESS 0x30

#define PROTO_VERSION 2

/* Registers that can be read by leader.
 */
enum {
    ADDR_SLOT = 10, // slot address (read only, 1 byte)
    ADDR_VERSION = 11, // protocol version (read only, 1 byte)
    ADDR_MATRIX_RAW = 12, // LED matrix (write only, 5 column bytes)
    ADDR_MATRIX_CHAR = 13, // LED matrix (write only, 1 bytes)
};

void i2c2_ev_isr (void)
{
    uint32_t sr1 = I2C_SR1 (I2C2);
    static uint8_t write_index = 0;
    static uint8_t write_buf[6];
    static uint8_t val;

    if ((sr1 & I2C_SR1_ADDR)) { // leader addresses follower
        (void)I2C_SR2 (I2C2);
        write_index = 0;
    }
    else if ((sr1 & I2C_SR1_RxNE)) { // leader sends byte to follower
        val = i2c_get_data (I2C2);
        if (write_index < sizeof (write_buf))
            write_buf[write_index++] = val;
    }
    else if ((sr1 & I2C_SR1_TxE) && !(sr1 & I2C_SR1_BTF)) { // xmit buf empty
        switch (write_buf[0]) {
            case ADDR_SLOT:
                val = address_get ();
                break;
            case ADDR_VERSION:
                val = PROTO_VERSION;
                break;
            default:
                val = 0;
                break;
        }
        i2c_send_data (I2C2, val);
    }
    else if ((sr1 & I2C_SR1_STOPF)) { // stop
        if (write_buf[0] == ADDR_MATRIX_RAW)
            matrix_set_from_isr (&write_buf[1]); // 5 bytes
        else if (write_buf[0] == ADDR_MATRIX_CHAR)
            matrix_set_char_from_isr (write_buf[1]);
        i2c_peripheral_enable(I2C2);
    }
    else if ((sr1 & I2C_SR1_AF)) { // ack failure
        I2C_SR1 (I2C2) &= ~(I2C_SR1_AF);
    }
}

void i2c_init (void)
{
    rcc_periph_clock_enable (RCC_AFIO);
    rcc_periph_clock_enable (RCC_GPIOB);
    rcc_periph_clock_enable (RCC_I2C2);

    nvic_set_priority (NVIC_I2C2_EV_IRQ, 12<<4);
    nvic_enable_irq (NVIC_I2C2_EV_IRQ);

    gpio_set_mode (GPIOB,
                   GPIO_MODE_OUTPUT_50_MHZ,
                   GPIO_CNF_OUTPUT_ALTFN_OPENDRAIN,
                   GPIO_I2C2_SCL | GPIO_I2C2_SDA); // PB10, PB11


    i2c_reset (I2C2);

    i2c_peripheral_disable (I2C2);
    i2c_set_speed (I2C2, i2c_speed_sm_100k, I2C_CR2_FREQ_36MHZ);
    i2c_set_own_7bit_slave_address (I2C2, FOLLOWER_ADDRESS);
    i2c_enable_interrupt (I2C2, I2C_CR2_ITEVTEN | I2C_CR2_ITBUFEN);
    i2c_peripheral_enable (I2C2);

    i2c_enable_ack (I2C2);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
