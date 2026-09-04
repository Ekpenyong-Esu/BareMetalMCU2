/**
 * @file    adc_hw.h
 * @brief   ADC hardware plumbing: clocks, DMA streams, parameter validation
 * @details Isolates everything that depends on which physical ADC instance is
 *          in use, so the layers above stay instance agnostic.
 */

#ifndef ADC_HW_H
#define ADC_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/**
 * @brief   Enable the peripheral clock for an ADC instance
 * @param   instance ADC1, ADC2 or ADC3
 * @retval  HAL_OK on success, HAL_ERROR for an unknown instance
 */
HAL_StatusTypeDef ADC_EnableInstanceClock(const ADC_TypeDef *instance);

/**
 * @brief   Configure and link the DMA stream that serves an ADC instance
 * @param   hadc Handle whose instance and DMA sub-handle are used
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ConfigureDma(ADC_HandleStruct *hadc);

/**
 * @brief   Clamp a resolution to a supported HAL constant
 * @param   resolution Requested resolution
 * @retval  uint32_t Supported resolution, defaulting to 12-bit
 */
uint32_t ADC_ValidateResolution(uint32_t resolution);

/**
 * @brief   Clamp a sampling time to a supported HAL constant
 * @param   sampling_time Requested sampling time
 * @retval  uint32_t Supported sampling time, defaulting to 84 cycles
 */
uint32_t ADC_ValidateSamplingTime(uint32_t sampling_time);

/**
 * @brief   Zero-based index of an ADC instance
 * @param   instance ADC1, ADC2 or ADC3
 * @retval  uint32_t Index, or ADC_INSTANCE_COUNT when unknown
 */
uint32_t ADC_InstanceIndex(const ADC_TypeDef *instance);

#ifdef __cplusplus
}
#endif

#endif /* ADC_HW_H */
