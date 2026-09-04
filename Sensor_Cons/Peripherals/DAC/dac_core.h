/**
 * @file dac_core.h
 * @brief DAC initialization and lifecycle
 *
 * STM32F4 DAC Initialization:
 * - Enables DAC clock (RCC_APB1ENR_DACEN)
 * - Configures the pin named in the config as analog output (GPIO_MODE_ANALOG)
 * - Initializes HAL DAC with the requested trigger, output buffer, alignment
 * - DAC_CHANNEL_1 or DAC_CHANNEL_2; the application says which pin carries it
 *   (PA4 for channel 1, PA5 for channel 2 on STM32F4)
 *
 * MSP (HAL_DAC_MspInit/MspDeInit):
 * - Implemented in dac.c (not weak HAL defaults) to ensure the clock is gated
 * - The output pin is handled by DAC_Init()/DAC_DeInit() from the config
 */

#ifndef DAC_CORE_H
#define DAC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dac_types.h"

/**
 * @brief   Initialize the DAC peripheral
 *
 * Validates the configuration (channel, output pin, trigger/buffer/alignment),
 * drives the output pin as analog, initializes the HAL DAC, configures the
 * channel, and marks the handle as initialized.
 *
 * @param   hdac Handle to populate (must be zeroed or uninitialized)
 * @param   config Requested configuration (channel, outPort/outPin, trigger,
 *                 output_buffer, alignment)
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid config or HAL failure
 */
HAL_StatusTypeDef DAC_Init(DAC_HandleStruct *hdac, const DAC_ConfigTypeDef *config);

/**
 * @brief   Deinitialize the DAC peripheral and release its clock and pin
 *
 * Calls HAL_DAC_DeInit() which invokes HAL_DAC_MspDeInit() to disable the
 * DAC clock, then returns the output pin to its reset state. Clears the
 * initialized flag.
 *
 * @param   hdac Handle
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR if not initialized or HAL fails
 */
HAL_StatusTypeDef DAC_DeInit(DAC_HandleStruct *hdac);

/**
 * @brief   Report whether the handle has been initialized
 *
 * @param   hdac Handle
 * @retval  bool true when initialized, false otherwise
 */
bool DAC_IsInitialized(const DAC_HandleStruct *hdac);

/**
 * @brief   Report whether a channel argument matches the initialized channel
 *
 * The HAL treats every value other than DAC_CHANNEL_1 as channel 2,
 * so an unchecked argument silently drives a pin this handle never
 * configured. This guard prevents that.
 *
 * @param   hdac Handle
 * @param   channel Channel supplied by the caller
 * @retval  bool true when the channel matches the one this handle was opened with
 */
bool DAC_IsChannelValid(const DAC_HandleStruct *hdac, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* DAC_CORE_H */
