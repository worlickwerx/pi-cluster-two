/* SPDX-License-Identifier: GPL-3.0-or-later */

/* matrix.c - 5x7 LED matrix attached to max7219 on SPI2
 *
 * Notes:
 * - 10K pullup on NSS (PB12)
 * - SN74AHCT125N converts 3V3 STM32 to 5V max7219
 * - Assumes anode column, cathode row matrix device.
 *   Caveat: this wastes memory: 7 bytes with 5 valid bits,
 *   not 5 bytes with 7 valid bits.
 */

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/cm3/nvic.h>

#include "matrix.h"
#include "address.h"
#include "font5x7.h"

#define MAX7219_MODE_DECODE       0x0900
#define MAX7219_MODE_INTENSITY    0x0A00
#define MAX7219_MODE_SCAN_LIMIT   0x0B00
#define MAX7219_MODE_SHUTDOWN     0x0C00
#define MAX7219_MODE_TEST         0x0F00
#define MAX7219_MODE_NOOP         0x0000

static void max7219_send (uint16_t data)
{
    spi_enable (SPI2); // assert NSS

    spi_write (SPI2, data);

    while (!(SPI_SR (SPI2) & SPI_SR_TXE)) // wait until transmit buffer empty
        taskYIELD ();
    while (SPI_SR (SPI2) & SPI_SR_BSY) // wait until not busy
        taskYIELD ();

    spi_disable (SPI2); // de-assert NSS
}

static void matrix_set_row (uint16_t row, uint8_t val)
{
    max7219_send ((row + 1) << 8 | val);
}

/* Set all rows to glyph encoded in 5 column bytes
 */
static void matrix_set_glyph (const uint8_t cols[5])
{
    uint8_t row, col;
    uint8_t val;

    for (row = 0; row < 7; row++) {
        val = 0;
        for (col = 0; col < 5; col++)
            val |= (cols[col] & 1<<row) ? 1<<col : 0;
        matrix_set_row (row, val);
    }
}

static void matrix_set_char (char c)
{
    if (c >= 0x20 && c <= 0x7f)
        matrix_set_glyph (&Font5x7[(c - 0x20) * 5]);
}

static void matrix_clear (void)
{
    uint8_t row;

    for (row = 0; row < 7; row++)
        matrix_set_row (row, 0);
}

/* Blink the entire display three times as a test
 */
static void matrix_selftest (void)
{
    uint8_t count;

    for (count = 1; count <= 6; count++) {
        max7219_send (MAX7219_MODE_TEST | (count & 1));
        vTaskDelay (pdMS_TO_TICKS (50));
    }
}

static void matrix_show_address (void)
{
    uint8_t addr = address_get ();

    matrix_set_char ('0' + addr / 10);
    vTaskDelay (pdMS_TO_TICKS (1000));
    matrix_set_char ('0' + addr % 10);
    vTaskDelay (pdMS_TO_TICKS (1000));
    matrix_clear ();
}

static void matrix_config (void)
{
    max7219_send (MAX7219_MODE_SCAN_LIMIT | 7); // mux over 8 digits/cols
    max7219_send (MAX7219_MODE_INTENSITY | 0xF);// intensity 0=dim F=bright
    max7219_send (MAX7219_MODE_DECODE | 0);     // no BCD decode
    max7219_send (MAX7219_MODE_SHUTDOWN | 1);   // power shutdown=0 normal=1
    max7219_send (MAX7219_MODE_TEST | 0);       // test mode off=0 on=1
    matrix_clear ();
}

static void matrix_task (void *args __attribute((unused)))
{
    matrix_config ();
    matrix_selftest ();
    matrix_show_address ();

    for (;;) {
        uint8_t row, col;

        for (row = 0; row < 7; row++) {
            for (col = 0; col < 5; col++) {
                matrix_set_row (row, 1<<col);
                vTaskDelay (pdMS_TO_TICKS (200));
            }
            matrix_set_row (row, 0);
        }
    }
}

void matrix_init (void)
{
    rcc_periph_clock_enable (RCC_GPIOB);
    rcc_periph_clock_enable (RCC_SPI2);

    gpio_set (GPIOB, GPIO12); // set NSS inactive
    gpio_set_mode (GPIOB,
                   GPIO_MODE_OUTPUT_50_MHZ,
                   GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                   GPIO12 | GPIO13 | GPIO15);   // NSS, SCK, MOSI
    spi_reset (SPI2);
    spi_init_master (SPI2,
                     SPI_CR1_BAUDRATE_FPCLK_DIV_8,
                     SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
                     SPI_CR1_CPHA_CLK_TRANSITION_2,
                     SPI_CR1_DFF_16BIT,
                     SPI_CR1_MSBFIRST);
    spi_disable_software_slave_management(SPI1);
    spi_enable_ss_output(SPI1);

    xTaskCreate (matrix_task,
                 "matrix",
                 200,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 NULL);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
