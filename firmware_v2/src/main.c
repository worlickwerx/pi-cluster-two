#include <stm32f0xx.h>

#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_rcc.h>
#include <stm32f0xx_hal_gpio.h>

int main(void)
{
    GPIO_InitTypeDef GPIO_Init;

    HAL_Init();

    __GPIOA_CLK_ENABLE();

    GPIO_Init.Pin = GPIO_PIN_5;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull = GPIO_PULLUP;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;

    HAL_GPIO_Init (GPIOA, &GPIO_Init);

    while (1) {
        //HAL_GPIO_TogglePin (GPIOA, GPIO_PIN_5);
        HAL_GPIO_WritePin (GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_Delay (500);
        HAL_GPIO_WritePin (GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_Delay (500);
    }
}

void SysTick_Handler (void)
{
    HAL_IncTick ();
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
