/**
 * @file    adc_channels.h
 * @brief   ADC channel to GPIO mapping
 * @details Owns the board-independent knowledge of which pin backs which
 *          channel, and configures that pin for analog input.
 *
 * STM32F4 ADC Channel Mapping:
 * - ADC1: PA0..PA7 (CH0..7), PB0..PB1 (CH8..9), PC0..PC5 (CH10..15)
 * - ADC2: Same pins as ADC1 (shared)
 * - ADC3: PF3..PF10 (CH0..7), PC0..PC3 (CH10..13)
 * - Internal channels (same on all ADCs):
 *   - CH16: Temperature sensor
 *   - CH17: VREFINT (internal 1.21V reference)
 *   - CH18: VBAT/2 (battery voltage through divider)
 *
 * This module configures the GPIO pin for analog mode (no pull, no speed).
 * Internal channels have no GPIO pin and succeed without touching GPIO.
 */

#ifndef ADC_CHANNELS_H
#define ADC_CHANNELS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"

/**
 * @brief   Put the pin backing @p channel into analog mode
 *
 * Configures the GPIO pin as analog input (GPIO_MODE_ANALOG, GPIO_NOPULL).
 * Uses the GPIO driver for pin initialization. Internal channels
 * (temperature, VREFINT, VBAT) have no pin and succeed without touching GPIO.
 *
 * @param   channel ADC channel (ADC_CHANNEL_0..18)
 * @retval  HAL_OK on success, HAL_ERROR when the channel is unknown or GPIO init fails
 */
HAL_StatusTypeDef ADC_ConfigureChannelGpio(uint32_t channel);

/**
 * @brief   Human readable name for a channel
 *
 * Returns the pin name (e.g., "PA0", "PC3") for external channels,
 * or the internal channel name ("TEMP", "VREFINT", "VBAT").
 *
 * @param   channel ADC channel
 * @retval  const char* Pin name, or "UNKNOWN"
 */
const char *ADC_GetChannelName(uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* ADC_CHANNELS_H */
