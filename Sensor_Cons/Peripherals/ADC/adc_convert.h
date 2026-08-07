/**
 * @file    adc_convert.h
 * @brief   ADC conversion control: polling, interrupt and DMA
 * @details All three transfer styles for starting conversions and collecting
 *          results. Knows nothing about pins, clocks or voltage scaling.
 */

#ifndef ADC_CONVERT_H
#define ADC_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/* Polling -------------------------------------------------------------------*/

/**
 * @brief   Start a software-triggered conversion
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_StartConversion(ADC_HandleStruct* hadc);

/**
 * @brief   Block until the current conversion finishes
 * @param   hadc ADC handle
 * @param   timeout_ms Timeout in milliseconds
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_PollForConversion(ADC_HandleStruct* hadc, uint32_t timeout_ms);

/**
 * @brief   Read the latest conversion result
 * @param   hadc ADC handle
 * @param   value Destination for the raw value
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_GetValue(ADC_HandleStruct* hadc, uint32_t* value);

/**
 * @brief   Configure, convert and read one channel
 * @param   hadc ADC handle
 * @param   channel Channel to read
 * @param   value Destination for the raw value
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ReadChannel(ADC_HandleStruct* hadc, uint32_t channel,
                                  uint32_t* value);

/* Continuous ----------------------------------------------------------------*/

/**
 * @brief   Start free-running conversions
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_StartContinuousConversion(ADC_HandleStruct* hadc);

/**
 * @brief   Stop free-running conversions
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_StopContinuousConversion(ADC_HandleStruct* hadc);

/* DMA -----------------------------------------------------------------------*/

/**
 * @brief   Stream conversions into a buffer
 * @param   hadc ADC handle
 * @param   buffer Destination buffer
 * @param   length Number of conversions
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_StartDMA(ADC_HandleStruct* hadc, uint32_t* buffer, uint32_t length);

/**
 * @brief   Stop a DMA stream
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_StopDMA(ADC_HandleStruct* hadc);

/**
 * @brief   Convert a scan sequence into a buffer and wait for it
 * @param   hadc ADC handle
 * @param   values Destination buffer
 * @param   num_channels Number of channels in the sequence
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ReadMultiChannel(ADC_HandleStruct* hadc, uint32_t* values,
                                       uint32_t num_channels);

/* Interrupt -----------------------------------------------------------------*/

/**
 * @brief   Start an interrupt-driven conversion
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_Start_IT(ADC_HandleStruct* hadc);

/**
 * @brief   Stop an interrupt-driven conversion
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_Stop_IT(ADC_HandleStruct* hadc);

/**
 * @brief   Configure a channel then start an interrupt-driven conversion
 * @param   hadc ADC handle
 * @param   channel Channel to read
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ReadChannel_IT(ADC_HandleStruct* hadc, uint32_t channel);

/**
 * @brief   Register the conversion complete callback
 * @param   hadc ADC handle
 * @param   callback Callback invoked from ISR context, NULL to clear
 */
void ADC_RegisterConvCompleteCallback(ADC_HandleStruct* hadc,
                                      void (*callback)(ADC_HandleStruct*, uint32_t));

/**
 * @brief   Register the error callback
 * @param   hadc ADC handle
 * @param   callback Callback invoked from ISR context, NULL to clear
 */
void ADC_RegisterErrorCallback(ADC_HandleStruct* hadc,
                               void (*callback)(ADC_HandleStruct*));

#ifdef __cplusplus
}
#endif

#endif /* ADC_CONVERT_H */
