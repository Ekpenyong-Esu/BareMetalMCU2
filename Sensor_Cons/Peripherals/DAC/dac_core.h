/**
 * @file dac_core.h
 * @brief DAC initialization and lifecycle
 */

#ifndef DAC_CORE_H
#define DAC_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dac_types.h"

/**
 * @brief   Initialize the DAC peripheral
 * @param   hdac Handle to populate
 * @param   config Requested configuration
 * @retval  HAL_StatusTypeDef HAL_OK on success
 * @note    Only DAC_CHANNEL_1 is supported; the MSP wires PA4 alone.
 */
HAL_StatusTypeDef DAC_Init(DAC_HandleStruct *hdac, const DAC_ConfigTypeDef *config);

/**
 * @brief   Deinitialize the DAC peripheral and release its clock and pin
 * @param   hdac Handle
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DAC_DeInit(DAC_HandleStruct *hdac);

/**
 * @brief   Report whether the handle has been initialized
 * @param   hdac Handle
 * @retval  bool true when initialized
 */
bool DAC_IsInitialized(const DAC_HandleStruct *hdac);

/**
 * @brief   Report whether a channel argument matches the initialized channel
 * @param   hdac Handle
 * @param   channel Channel supplied by the caller
 * @retval  bool true when the channel may be used with this handle
 * @note    The HAL treats every value other than DAC_CHANNEL_1 as channel 2,
 *          so an unchecked argument silently drives an unconfigured pin.
 */
bool DAC_IsChannelValid(const DAC_HandleStruct *hdac, uint32_t channel);

#ifdef __cplusplus
}
#endif

#endif /* DAC_CORE_H */
