/**
 ******************************************************************************
 * @file    dht_timing.c
 * @brief   Microsecond time base for the DHT driver - internal
 ******************************************************************************
 */

#include "dht_timing.h"

#include "stm32f4xx.h"

#define DHT_US_PER_SECOND 1000000U

static uint32_t DHT_Timing_TicksPerUs(void) {
    uint32_t ticks = SystemCoreClock / DHT_US_PER_SECOND;

    /* A zero divisor here would make every timeout expire instantly. */
    return (ticks == 0U) ? 1U : ticks;
}

bool DHT_Timing_Init(void) {
    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0U) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    }
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    /* Every delay below spins on CYCCNT, so a stalled counter would hang. */
    return DWT->CYCCNT != 0U;
}

uint32_t DHT_Timing_Now(void) {
    return DWT->CYCCNT;
}

uint32_t DHT_Timing_ElapsedUs(uint32_t startTicks) {
    return (DWT->CYCCNT - startTicks) / DHT_Timing_TicksPerUs();
}

void DHT_Timing_DelayUs(uint32_t micros) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = DHT_Timing_TicksPerUs() * micros;

    while ((DWT->CYCCNT - start) < ticks) {
        __NOP();
    }
}
