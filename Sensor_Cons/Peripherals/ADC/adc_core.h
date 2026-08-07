/**
 * @file    adc_core.h
 * @brief   ADC lifecycle, channel configuration and handle registry
 * @details Owns initialization and the instance to handle registry that lets
 *          the HAL callbacks find the right handle. Replaces the former global
 *          hadc1 instance, so ADC1, ADC2 and ADC3 can be used together.
 */

#ifndef ADC_CORE_H
#define ADC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/**
 * @brief   Initialize an ADC handle
 * @param   hadc Handle to initialize
 * @param   config Configuration to apply; config->instance selects the
 *          peripheral and defaults to ADC1 when NULL
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_Init(ADC_HandleStruct* hadc, const ADC_ConfigTypeDef* config);

/**
 * @brief   Release an ADC handle and unregister it
 * @param   hadc Handle to release
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_DeInit(ADC_HandleStruct* hadc);

/**
 * @brief   Configure a single channel as the conversion sequence
 * @param   hadc ADC handle
 * @param   channel Channel to convert
 * @param   sampling_time Sampling time for that channel
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ConfigChannel(ADC_HandleStruct* hadc, uint32_t channel,
                                    uint32_t sampling_time);

/**
 * @brief   Configure a scan sequence of several channels
 * @param   hadc ADC handle
 * @param   channels Channels in conversion order
 * @param   sampling_times Sampling time per channel
 * @param   num_channels Number of entries in both arrays
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_ConfigMultiChannel(ADC_HandleStruct* hadc,
                                         const uint32_t* channels,
                                         const uint32_t* sampling_times,
                                         uint32_t num_channels);

/**
 * @brief   Change the conversion resolution
 * @param   hadc ADC handle
 * @param   resolution New resolution
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_SetResolution(ADC_HandleStruct* hadc, uint32_t resolution);

/**
 * @brief   Change the sampling time of a channel
 * @param   hadc ADC handle
 * @param   channel Channel to adjust
 * @param   sampling_time New sampling time
 * @retval  HAL_StatusTypeDef Status of the operation
 */
HAL_StatusTypeDef ADC_SetSamplingTime(ADC_HandleStruct* hadc, uint32_t channel,
                                      uint32_t sampling_time);

/**
 * @brief   Current driver status
 * @param   hadc ADC handle
 * @retval  HAL_StatusTypeDef HAL_OK, HAL_BUSY or HAL_ERROR
 */
HAL_StatusTypeDef ADC_GetStatus(const ADC_HandleStruct* hadc);

/**
 * @brief   Whether the handle is initialized
 * @param   hadc ADC handle
 * @retval  bool true when usable
 */
bool ADC_IsReady(const ADC_HandleStruct* hadc);

/**
 * @brief   Whether a conversion has finished
 * @param   hadc ADC handle
 * @retval  bool true when the end-of-conversion flag is set
 */
bool ADC_IsConversionComplete(const ADC_HandleStruct* hadc);

/**
 * @brief   Mark a handle unusable after a fatal error
 * @param   hadc ADC handle
 */
void ADC_ErrorHandler(ADC_HandleStruct* hadc);

/**
 * @brief   Resolve the driver handle that owns a HAL handle
 * @param   hal HAL handle supplied by a HAL callback
 * @retval  ADC_HandleStruct* Owning handle, or NULL when unregistered
 * @note    Used by the HAL callbacks and by Core/Src/stm32f4xx_it.c
 */
ADC_HandleStruct* ADC_GetHandleFor(const ADC_HandleTypeDef* hal);

/**
 * @brief   Human readable form of a HAL status
 * @param   status Status to describe
 * @retval  const char* Status name
 */
const char* ADC_GetStatusString(HAL_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* ADC_CORE_H */
