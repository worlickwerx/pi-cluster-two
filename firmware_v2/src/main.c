#include <stm32f1xx.h>

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_rcc.h>
#include <stm32f1xx_hal_gpio.h>

#include "debug.h"
#include "address.h"
#include "activity.h"

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

void mcu_setup (void)
{
    if (configure_clock () < 0)
        FATAL ("configure_clock failed\n");
    SystemCoreClockUpdate ();
    HAL_Init();

    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();
    //__GPIOC_CLK_ENABLE();
}

int main (void)
{
    uint8_t mod, node;

    mcu_setup ();
    address_setup ();
    address_get (&mod, &node);
    activity_setup ();

    while (1) {
        activity_update ();
        if (HAL_GetTick () % 500 == 0)
            activity_pulse ();
    }

    activity_finalize ();
    address_finalize ();
}

void SysTick_Handler (void)
{
    HAL_IncTick ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
