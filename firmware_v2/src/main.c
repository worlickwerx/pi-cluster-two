#include <stm32f1xx.h>

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_gpio.h>

#include "debug.h"

/* Configure HSE clock with 8MHz xtal -> PLL (x9) -> 72 MHz system clock.
 */
int configure_clock (void)
{
    RCC_OscInitTypeDef o;
    RCC_ClkInitTypeDef c;

    o.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    o.HSEState = RCC_HSE_ON;
    o.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    o.LSEState = RCC_LSE_ON;
    o.HSIState = RCC_HSI_OFF;
    o.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    o.LSIState = RCC_LSI_OFF;

    o.PLL.PLLState = RCC_PLL_ON;
    o.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    o.PLL.PLLMUL = RCC_PLL_MUL9;

    if (HAL_RCC_OscConfig (&o) != HAL_OK)
        return -1;

    c.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK
                                       | RCC_CLOCKTYPE_PCLK1
                                       | RCC_CLOCKTYPE_PCLK2;
    c.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    c.AHBCLKDivider = RCC_SYSCLK_DIV1;
    c.APB2CLKDivider = RCC_HCLK_DIV1;
    c.APB1CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig (&c, FLASH_LATENCY_2) != HAL_OK)
        return -1;
    return 0;
}

int main (void)
{
    GPIO_InitTypeDef GPIO_Init;

    if (configure_clock () < 0)
        FATAL ("configure_clock failed\n");
    SystemCoreClockUpdate ();
    HAL_Init();

    __GPIOB_CLK_ENABLE();

    // PB12 = blue LED
    GPIO_Init.Pin = GPIO_PIN_12;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init (GPIOB, &GPIO_Init);


    int count = 0;
    while (1) {
        itm_printf ("Blink: %d\n", count++);
        HAL_GPIO_WritePin (GPIOB, GPIO_PIN_12, GPIO_PIN_RESET); // ON
        HAL_Delay (200);
        HAL_GPIO_WritePin (GPIOB, GPIO_PIN_12, GPIO_PIN_SET); // OFF
        HAL_Delay (200);
    }
}
void SysTick_Handler (void)
{
    HAL_IncTick ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
