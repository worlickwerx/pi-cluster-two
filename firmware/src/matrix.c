/* SPDX-License-Identifier: GPL-3.0-or-later */

/* matrix.c - 5x7 LED matrix attached to max7219 on SPI2
 *
 * Notes:
 * - 10K pullup on NSS (PB12)
 * - Use software NSS management to avoid conflict with I2C2 SMBA
 *   (see Errata ES096 Rev 14 sec 2.9.6)
 * - SN74AHCT125N converts 3V3 STM32 to 5V max7219
 * - Assumes anode column, cathode row matrix device.
 * - Discrete green and red front panel LEDs are wired on row 7.
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

/* G F E D C B A dp
   0 1 2 3 4 5 6 7
 -------------------
0| * * * * *       |
1| * * * * *       |
2| * * * * *       |
3| * * * * *       |
4| * * * * *       |
5| * * * * *       |
6| * * * * *       |
7|           r g   |
 -------------------
*/

/* Frame buffer
 * Can be updated while matrix_task is syncing the display.
 */
static volatile uint8_t pending[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

static TaskHandle_t matrix_task_handle = NULL;
static TaskHandle_t green_task_handle = NULL;

/* Set a byte to the chip over SPI.
 * This function may sleep.
 */
static void max7219_send (uint16_t data)
{
    gpio_clear (GPIOB, GPIO12);

    spi_write (SPI2, data);

    while (!(SPI_SR (SPI2) & SPI_SR_TXE)) // wait until transmit buffer empty
        taskYIELD ();
    while (SPI_SR (SPI2) & SPI_SR_BSY) // wait until not busy
        taskYIELD ();

    gpio_set (GPIOB, GPIO12);
}

/* Set display 'row' (0-7) to 'val'.
 * This function may sleep.
 */
static void max7219_send_row (uint16_t row, uint8_t val)
{
    if (row < 8)
        max7219_send ((row + 1) << 8 | val);
}

/* Sync display to match 'pending' frame buffer.
 * If 'force' is true, update all rows.
 * This function may sleep.
 */
static void max7219_sync (bool force)
{
    static uint8_t visible[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    uint8_t row;
    uint8_t update = 0;

    taskENTER_CRITICAL ();
    for (row = 0; row < 8; row++) {
        if (force || pending[row] != visible[row]) {
            update |= (1<<row);
            visible[row] = pending[row];
        }
    }
    taskEXIT_CRITICAL ();

    for (row = 0; row < 8; row++) {
        if (update & (1<<row))
            max7219_send_row (row, visible[row]);
    }
}

/* Blink the entire display three times as a test
 * This function may sleep.
 */
static void max7219_selftest (void)
{
    uint8_t count;

    for (count = 1; count <= 6; count++) {
        max7219_send (MAX7219_MODE_TEST | (count & 1));
        vTaskDelay (pdMS_TO_TICKS (50));
    }
}

/* Configure the chip.
 * This function may sleep.
 */
static void max7219_config (void)
{
    max7219_send (MAX7219_MODE_SCAN_LIMIT | 7); // mux over 8 digits/cols
    max7219_send (MAX7219_MODE_INTENSITY | 0xF);// intensity 0=dim F=bright
    max7219_send (MAX7219_MODE_DECODE | 0);     // no BCD decode
    max7219_send (MAX7219_MODE_SHUTDOWN | 1);   // power shutdown=0 normal=1
    max7219_send (MAX7219_MODE_TEST | 0);       // test mode off=0 on=1
    max7219_sync (true);
}

/* Notify matrix_task() to perform update.
 */
static void matrix_sync (void)
{
    xTaskNotifyGive (matrix_task_handle);
}

static void matrix_sync_from_isr (void)
{
    vTaskNotifyGiveFromISR (matrix_task_handle, NULL);
}

static void matrix_xfrm (const uint8_t cols[], volatile uint8_t rows[])
{
    uint8_t row, col;

    for (row = 0; row < 7; row++) {
        rows[row] = 0;
        for (col = 0; col < 5; col++)
            rows[row] |= (cols[col] & 1<<row) ? 1<<col : 0;
    }
}

/* Set complete 5x7 matrix, using column major array.
 * Column bytes pack better than row bytes, so do the bit level gymnastics
 * to convert from that more compact representation to frame buffer rows.
 */
void matrix_set (const uint8_t cols[])
{
    taskENTER_CRITICAL ();
    matrix_xfrm (cols, pending);
    taskEXIT_CRITICAL ();
    matrix_sync ();
}

void matrix_set_from_isr (const uint8_t cols[])
{
    UBaseType_t status;

    status = taskENTER_CRITICAL_FROM_ISR ();
    matrix_xfrm (cols, pending);
    taskEXIT_CRITICAL_FROM_ISR (status);
    matrix_sync_from_isr ();
}

/* Put character to 5x7 matrix.
 */
void matrix_set_char (char c)
{
    if (c >= 0x20 && c <= 0x7f)
        matrix_set (&Font5x7[(c - 0x20) * 5]);
}
void matrix_set_char_from_isr (char c)
{
    if (c >= 0x20 && c <= 0x7f)
        matrix_set_from_isr (&Font5x7[(c - 0x20) * 5]);
}

void matrix_set_red (uint8_t val)
{
    taskENTER_CRITICAL ();
    if (val)
        pending[7] |= (1<<5);
    else
        pending[7] &= ~(1<<5);
    taskEXIT_CRITICAL ();
    matrix_sync ();
}

/* Only used internally - externally use matrix_pulse_green()
 */
static void matrix_set_green (uint8_t val)
{
    taskENTER_CRITICAL ();
    if (val)
        pending[7] |= (1<<6);
    else
        pending[7] &= ~(1<<6);
    taskEXIT_CRITICAL ();
    matrix_sync ();
}

/* Perform max7219 initialization, then clock the frame buffer out over SPI
 * when requested by matrix_sync().
 */
static void matrix_task (void *args __attribute((unused)))
{
    max7219_config ();
    max7219_selftest ();

    for (;;) {
        ulTaskNotifyTake (pdTRUE, portMAX_DELAY);
        max7219_sync (false);
    }
}

/* Pulse the green led quickly when notified via matrix_pulse_green().
 */
static void green_task (void *args __attribute((unused)))
{
    for (;;) {
        ulTaskNotifyTake (pdTRUE, portMAX_DELAY);

        matrix_set_green (1);
        vTaskDelay (pdMS_TO_TICKS (5));
        matrix_set_green (0);
    }
}

void matrix_pulse_green (void)
{
    xTaskNotifyGive (green_task_handle);
}

void matrix_init (void)
{
    rcc_periph_clock_enable (RCC_AFIO);
    rcc_periph_clock_enable (RCC_GPIOB);
    rcc_periph_clock_enable (RCC_SPI2);

    gpio_set (GPIOB, GPIO12);
    gpio_set_mode (GPIOB,
                   GPIO_MODE_OUTPUT_2_MHZ,
                   GPIO_CNF_OUTPUT_OPENDRAIN, // ~NSS has an ext. 10K pullup
                   GPIO12);

    gpio_set_mode (GPIOB,
                   GPIO_MODE_OUTPUT_50_MHZ,
                   GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
                   GPIO13 | GPIO15);   // SCK, MOSI

    spi_reset (SPI2);
    spi_init_master (SPI2,
                     SPI_CR1_BAUDRATE_FPCLK_DIV_8,
                     SPI_CR1_CPOL_CLK_TO_1_WHEN_IDLE,
                     SPI_CR1_CPHA_CLK_TRANSITION_2,
                     SPI_CR1_DFF_16BIT,
                     SPI_CR1_MSBFIRST);
    spi_enable_software_slave_management (SPI2);
    spi_set_nss_high (SPI2);
    spi_enable (SPI2);

    xTaskCreate (matrix_task,
                 "matrix",
                 200,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 &matrix_task_handle);

    xTaskCreate (green_task,
                 "green",
                 200,
                 NULL,
                 configMAX_PRIORITIES - 1,
                 &green_task_handle);
}

/*
 * vi:ts=4 sw=4 expandtab
 */
