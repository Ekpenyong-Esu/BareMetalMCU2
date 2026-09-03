/**
 * @file dac_core.h
 * @brief DAC initialization and lifecycle
 *
 * STM32F4 DAC Initialization:
 * - Enables DAC clock (RCC_APB1ENR_DACEN)
 * - Configures PA4 as analog output (GPIO_MODE_ANALOG)
 * - Initializes HAL DAC with the requested trigger, output buffer, alignment
 * - Only DAC_CHANNEL_1 is supported; channel 2 (PA5) is not wired on this board
 *
 * MSP (HAL_DAC_MspInit/MspDeInit):
 * - Implemented in dac.c (not weak HAL defaults) to ensure proper pin/clock management
 * - MspInit: enables DAC clock, configures PA4 as analog
 * - MspDeInit: disables DAC clock, resets PA4 to analog (reset state)
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
 * Validates the configuration (only CH1, valid trigger/buffer/alignment),
 * initializes the HAL DAC, configures the channel, and marks the handle
 * as initialized. The MSP (clock + GPIO) is handled by HAL_DAC_MspInit()
 * in dac.c.
 *
 * @param   hdac Handle to populate (must be zeroed or uninitialized)
 * @param   config Requested configuration (channel, trigger, output_buffer, alignment)
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid config or HAL failure
 * @note    Only DAC_CHANNEL_1 is supported; the MSP wires PA4 alone.
 */
HAL_StatusTypeDef DAC_Init(DAC_HandleStruct *hdac, const DAC_ConfigTypeDef *config);

/**
 * @brief   Deinitialize the DAC peripheral and release its clock and pin
 *
 * Calls HAL_DAC_DeInit() which invokes HAL_DAC_MspDeInit() to disable the
 * DAC clock and return PA4 to its reset state. Clears the initialized flag.
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
 * so an unchecked argument silently drives an unconfigured pin (PA5).
 * This guard prevents that.
 *
 * @param   hdac Handle
 * @param   channel Channel supplied by the caller
 * @retval  bool true when the channel may be used with this handle (must be CH1)
 * @note    The HAL treats every value other than DAC_CHANNEL_1 as channel 2,
 *          so an unchecked argument silently drives an unconfigured pin.
 */
bool DAC_IsChannelValid(const DAC_HandleStruct *hdac, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* DAC_CORE_H */
