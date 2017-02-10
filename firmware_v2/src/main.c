#include <stm32f0xx.h>

#include <stm32f0xx_hal.h>
#include <stm32f0xx_hal_rcc.h>
#include <stm32f0xx_hal_gpio.h>

int main(void)
{
    GPIO_InitTypeDef GPIO_Init;

    HAL_Init();

    __GPIOA_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();

    // PA5 = nucleo LD2
    GPIO_Init.Pin = GPIO_PIN_5;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init (GPIOA, &GPIO_Init);

    // PC5 = PWR-ON
    GPIO_Init.Pin = GPIO_PIN_5;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init (GPIOC, &GPIO_Init);

    // PC6 = CAN-ACT
    GPIO_Init.Pin = GPIO_PIN_6;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init (GPIOC, &GPIO_Init);

    // PC7 = IDENTIFY
    GPIO_Init.Pin = GPIO_PIN_7;
    GPIO_Init.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init (GPIOC, &GPIO_Init);

    // set initial PWR-ON state
    HAL_GPIO_WritePin (GPIOC, GPIO_PIN_5, GPIO_PIN_RESET);

    while (1) {
        HAL_GPIO_WritePin (GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
        HAL_GPIO_WritePin (GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_GPIO_WritePin (GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);
        HAL_Delay (500);
        HAL_GPIO_WritePin (GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
        HAL_GPIO_WritePin (GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_GPIO_WritePin (GPIOC, GPIO_PIN_7, GPIO_PIN_SET);
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
