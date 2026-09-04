/**
 * @file    adc_core.h
 * @brief   Start, stop and set up ADC
 * @details This file handles starting and stopping the ADC and
 *          picking which pins to read from.
 *
 * How it works (in simple words):
 * - Init turns on the ADC, sets the pin to analog, and saves the handle.
 * - You can read one pin or many pins in a row.
 * - You can change how detailed the reading is or how long it samples.
 * - You can ask if the ADC is ready or if a reading is done.
 */

#ifndef ADC_CORE_H
#define ADC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/**
 * @brief   Start the ADC and get it ready
 * @param   hadc Empty handle to fill in
 * @param   config Settings to use (pick ADC1/ADC2/ADC3, channel, etc.)
 * @retval  HAL_OK if it worked, HAL_ERROR if not
 */
HAL_StatusTypeDef ADC_Init(ADC_HandleStruct *hadc, const ADC_ConfigTypeDef *config);

/**
 * @brief   Stop the ADC and clean up
 * @param   hadc Handle to stop
 * @retval  HAL_OK if it worked, HAL_ERROR if not
 */
HAL_StatusTypeDef ADC_DeInit(ADC_HandleStruct *hadc);

/**
 * @brief   Pick one pin to read from
 * @param   hadc ADC handle (must be started)
 * @param   channel Which pin/channel to read
 * @param   sampling_time How long to sample that pin
 * @retval  HAL_OK if it worked, HAL_ERROR if not
 */
HAL_StatusTypeDef ADC_ConfigChannel(ADC_HandleStruct *hadc, uint32_t channel,
                                    uint32_t sampling_time);

/**
 * @brief   Pick many pins to read one after another
 * @param   hadc ADC handle (must be started)
 * @param   channels List of pins to read, in order
 * @param   sampling_times How long to sample each pin
 * @param   num_channels How many pins in the lists
 * @retval  HAL_OK if it worked, HAL_ERROR if not
 */
HAL_StatusTypeDef ADC_ConfigMultiChannel(ADC_HandleStruct *hadc, const uint32_t *channels,
                                         const uint32_t *sampling_times, uint32_t num_channels);

/**
 * @brief   Change how detailed the reading is
 * @param   hadc ADC handle (must be started)
 * @param   resolution New detail level (12, 10, 8 or 6 bit)
 * @retval  HAL_OK if it worked, HAL_ERROR if not
 */
HAL_StatusTypeDef ADC_SetResolution(ADC_HandleStruct *hadc, uint32_t resolution);

/**
 * @brief   Change how long to sample a pin
 * @param   hadc ADC handle (must be started)
 * @param   channel Which pin to change
 * @param   sampling_time New sample time
 * @retval  HAL_OK if it worked, HAL_ERROR if not
 */
HAL_StatusTypeDef ADC_SetSamplingTime(ADC_HandleStruct *hadc, uint32_t channel,
                                      uint32_t sampling_time);

/**
 * @brief   Check the current status
 * @param   hadc ADC handle
 * @retval  HAL_OK if ready, HAL_BUSY if reading, HAL_ERROR if not ready
 */
HAL_StatusTypeDef ADC_GetStatus(const ADC_HandleStruct *hadc);

/**
 * @brief   Is the ADC ready to use?
 * @param   hadc ADC handle
 * @retval  true if ready, false if not
 */
bool ADC_IsReady(const ADC_HandleStruct *hadc);

/**
 * @brief   Has the reading finished?
 * @param   hadc ADC handle
 * @retval  true if a new reading is ready
 */
bool ADC_IsConversionComplete(const ADC_HandleStruct *hadc);

/**
 * @brief   Mark the ADC as broken after an error
 * @details After this, all other calls will fail until you start again.
 * @param   hadc ADC handle
 */
void ADC_ErrorHandler(ADC_HandleStruct *hadc);

/**
 * @brief   Find our handle from a low-level handle
 * @param   hal Low-level handle given by the system
 * @retval  Our handle, or NULL if not found
 */
ADC_HandleStruct *ADC_GetHandleFor(const ADC_HandleTypeDef *hal);

/**
 * @brief   Turn a status code into words
 * @param   status Code to explain
 * @retval  Text like "OK" or "ERROR"
 */
const char *ADC_GetStatusString(HAL_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* ADC_CORE_H */
