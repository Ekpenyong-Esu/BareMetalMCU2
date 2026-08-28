/**
 * @file    adc_clock.c
 * @brief   ADC clock enable and rate query
 */

#include "adc_clock.h"

#include <stddef.h>

/* ========================== Clock enable ========================== */

bool ADC_Clock_Enable(const ADC_TypeDef* instance)
{
    if (instance == ADC1) { __HAL_RCC_ADC1_CLK_ENABLE(); return true; }
    if (instance == ADC2) { __HAL_RCC_ADC2_CLK_ENABLE(); return true; }
    if (instance == ADC3) { __HAL_RCC_ADC3_CLK_ENABLE(); return true; }

    return false;
}

bool ADC_Clock_Disable(const ADC_TypeDef* instance)
{
    if (instance == ADC1) { __HAL_RCC_ADC1_CLK_DISABLE(); return true; }
    if (instance == ADC2) { __HAL_RCC_ADC2_CLK_DISABLE(); return true; }
    if (instance == ADC3) { __HAL_RCC_ADC3_CLK_DISABLE(); return true; }

    return false;
}

/* ========================== Rate query ========================== */

uint32_t ADC_Clock_GetHz(const ADC_TypeDef* instance)
{
    if ((instance != ADC1) && (instance != ADC2) && (instance != ADC3)) {
        return 0U;
    }

    /* All three converters share one prescaler in the common control register.
     * The two ADCPRE bits select /2, /4, /6 or /8, i.e. 2 * (field + 1). */
    uint32_t adcpre = (ADC->CCR & ADC_CCR_ADCPRE) >> ADC_CCR_ADCPRE_Pos;

    return HAL_RCC_GetPCLK2Freq() / (2U * (adcpre + 1U));
}

uint32_t ADC_Clock_SamplingCycles(uint32_t sampling_time)
{
    switch (sampling_time) {
        case ADC_SAMPLETIME_3CYCLES:   return 3U;
        case ADC_SAMPLETIME_15CYCLES:  return 15U;
        case ADC_SAMPLETIME_28CYCLES:  return 28U;
        case ADC_SAMPLETIME_56CYCLES:  return 56U;
        case ADC_SAMPLETIME_84CYCLES:  return 84U;
        case ADC_SAMPLETIME_112CYCLES: return 112U;
        case ADC_SAMPLETIME_144CYCLES: return 144U;
        case ADC_SAMPLETIME_480CYCLES: return 480U;
        default:                       return 84U;
    }
}

uint32_t ADC_Clock_SamplingTimeFor(const ADC_TypeDef* instance, uint32_t min_us)
{
    static const uint32_t kLadder[] = {
        ADC_SAMPLETIME_3CYCLES,   ADC_SAMPLETIME_15CYCLES,
        ADC_SAMPLETIME_28CYCLES,  ADC_SAMPLETIME_56CYCLES,
        ADC_SAMPLETIME_84CYCLES,  ADC_SAMPLETIME_112CYCLES,
        ADC_SAMPLETIME_144CYCLES, ADC_SAMPLETIME_480CYCLES,
    };
    static const uint32_t kLadderCount = sizeof(kLadder) / sizeof(kLadder[0]);

    uint32_t adcHz = ADC_Clock_GetHz(instance);
    if (adcHz == 0U) {
        return ADC_SAMPLETIME_480CYCLES; /* unknown clock: sample as long as possible */
    }

    /* cycles needed = min_us * f_adc / 1e6, rounded up so the result is never
     * short of the datasheet minimum. */
    uint32_t needed = (uint32_t)((((uint64_t)min_us * (uint64_t)adcHz) + 999999U) / 1000000U);

    for (uint32_t i = 0; i < kLadderCount; i++) {
        if (ADC_Clock_SamplingCycles(kLadder[i]) >= needed) {
            return kLadder[i];
        }
    }

    /* Unreachable at this clock; the longest setting is the closest we get. */
    return ADC_SAMPLETIME_480CYCLES;
}
