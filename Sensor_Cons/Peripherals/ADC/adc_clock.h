/**
 * @file    adc_clock.h
 * @brief   ADC clock enable and rate query
 * @details Single-responsibility module answering the two questions every ADC
 *          user has before configuring anything: how do I turn this ADC on,
 *          and how fast is the clock feeding it?
 *
 *          The rate matters because the datasheet states minimum sampling
 *          times in microseconds while the peripheral is programmed in ADC
 *          clock cycles. ADC_Clock_SamplingTimeFor() bridges the two, so
 *          callers state what the datasheet says rather than hand-computing
 *          cycle counts that silently break when the clock tree changes.
 *
 * @note    Mirrors tim_clock.h, which plays the same role for the timers.
 */

#ifndef ADC_CLOCK_H
#define ADC_CLOCK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/**
 * @brief   Enable the peripheral clock of an ADC
 * @param   instance ADC1, ADC2 or ADC3
 * @retval  bool true on success, false if the instance is not present
 */
bool ADC_Clock_Enable(const ADC_TypeDef* instance);

/**
 * @brief   Disable the peripheral clock of an ADC
 * @param   instance ADC1, ADC2 or ADC3
 * @retval  bool true on success, false if the instance is not present
 */
bool ADC_Clock_Disable(const ADC_TypeDef* instance);

/**
 * @brief   Get the clock actually driving the ADC conversions
 * @note    Not the same as the APB2 bus clock: the common ADC prescaler
 *          divides PCLK2 by 2, 4, 6 or 8 before it reaches the converters.
 * @param   instance ADC peripheral (all three share the common prescaler)
 * @retval  uint32_t ADC clock in Hz, or 0 for an unknown instance
 */
uint32_t ADC_Clock_GetHz(const ADC_TypeDef* instance);

/**
 * @brief   Number of ADC clock cycles a sampling time constant represents
 * @param   sampling_time HAL ADC_SAMPLETIME_* constant
 * @retval  uint32_t Cycle count, defaulting to 84 for an unknown constant
 */
uint32_t ADC_Clock_SamplingCycles(uint32_t sampling_time);

/**
 * @brief   Shortest sampling time constant that samples for at least @p min_us
 * @details The datasheet quotes minimum sampling times in microseconds for the
 *          internal sensors and for high-impedance sources; this converts such
 *          a requirement into the cycle constant to program, given the clock
 *          actually feeding the converter.
 * @param   instance ADC peripheral
 * @param   min_us Minimum sampling duration in microseconds
 * @retval  uint32_t HAL ADC_SAMPLETIME_* constant; the longest available
 *          (480 cycles) when @p min_us is unreachable at this clock
 */
uint32_t ADC_Clock_SamplingTimeFor(const ADC_TypeDef* instance, uint32_t min_us);

#ifdef __cplusplus
}
#endif

#endif /* ADC_CLOCK_H */
