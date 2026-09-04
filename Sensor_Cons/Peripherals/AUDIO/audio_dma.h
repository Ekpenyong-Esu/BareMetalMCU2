/**
 ******************************************************************************
 * @file    audio_dma.h
 * @brief   Transmit DMA setup shared by the audio backends
 * @details The stream and channel come from the handle's config and the
 *          NVIC line is derived from the stream, so this module never needs
 *          to know whether it is feeding SAI or I2S.
 ******************************************************************************
 */

#ifndef AUDIO_DMA_H
#define AUDIO_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_types.h"

/**
 * @brief   Configure and start the transmit DMA named in the config
 * @details Enables the controller clock, initialises the stream in circular
 *          memory-to-peripheral mode, hands the handle to the backend for
 *          linking and unmasks the stream interrupt.
 * @param   dev Handle whose config selects the stream and channel
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef Audio_DmaInit(AUDIO_Handle_t *dev);

/**
 * @brief   Mask the stream interrupt and release the DMA stream
 * @param   dev Device to tear down
 * @retval  None
 */
void Audio_DmaDeInit(AUDIO_Handle_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_DMA_H */
