/**
 * @file ir_timing.c
 * @brief IR timing conversions and microsecond delay
 */

/* Includes ------------------------------------------------------------------*/
#include "ir_timing.h"

/* Private define ------------------------------------------------------------*/
#define IR_MICROSECONDS_PER_SECOND  1000000U

/* Public functions ----------------------------------------------------------*/

void IR_TimingInit(void)
{
    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0U)
    {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }

    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void IR_DelayUs(uint32_t us)
{
    const uint32_t start = DWT->CYCCNT;
    const uint32_t ticks = (SystemCoreClock / IR_MICROSECONDS_PER_SECOND) * us;

    while ((DWT->CYCCNT - start) < ticks)
    {
        __NOP();
    }
}

uint32_t IR_MicrosecondsToTicks(uint32_t microseconds, uint32_t timerFreq)
{
    if (timerFreq == 0U)
    {
        return 0U;
    }

    /* A 9 ms header at 84 MHz is 756000 ticks, and the intermediate product
       overflows 32 bits well before that, so widen the multiplication. */
    return (uint32_t)(((uint64_t)microseconds * timerFreq) / IR_MICROSECONDS_PER_SECOND);
}

uint32_t IR_TicksToMicroseconds(uint32_t ticks, uint32_t timerFreq)
{
    if (timerFreq == 0U)
    {
        return 0U;
    }

    return (uint32_t)(((uint64_t)ticks * IR_MICROSECONDS_PER_SECOND) / timerFreq);
}

bool IR_IsWithinTolerance(uint32_t measured, uint32_t expected, uint32_t tolerance)
{
    /* Computing expected - tolerance first would wrap when the tolerance is the
       larger of the two and make every comparison fail. */
    const uint32_t difference = (measured > expected) ? (measured - expected)
                                                      : (expected - measured);

    return difference <= tolerance;
}
