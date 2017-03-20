#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>

static inline uint32_t timesince (uint32_t t0)
{
    uint32_t t1 = HAL_GetTick ();

    if (t1 < t0)
        return 0xFFFFFFFF - t0 + t1;
    else
        return t1 - t0;
}

/*
 * vi:tabstop=4 shiftwidth=4 expandtab
 */
