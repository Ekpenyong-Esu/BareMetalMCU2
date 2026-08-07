/**
 * @file    adc_channels.h
 * @brief   ADC channel to GPIO mapping
 * @details Owns the board-independent knowledge of which pin backs which
 *          channel, and configures that pin for analog input.
 */

#ifndef ADC_CHANNELS_H
#define ADC_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/**
 * @brief   Put the pin backing @p channel into analog mode
 * @param   channel ADC channel
 * @retval  HAL_OK on success, HAL_ERROR when the channel is unknown
 * @note    Internal channels (temperature, VREFINT, VBAT) have no pin and
 *          succeed without touching GPIO.
 */
HAL_StatusTypeDef ADC_ConfigureChannelGpio(uint32_t channel);

/**
 * @brief   Human readable name for a channel
 * @param   channel ADC channel
 * @retval  const char* Pin name, or "UNKNOWN"
 */
const char* ADC_GetChannelName(uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* ADC_CHANNELS_H */
