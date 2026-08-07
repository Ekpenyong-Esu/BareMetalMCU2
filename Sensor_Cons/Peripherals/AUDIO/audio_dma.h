/**
  ******************************************************************************
  * @file    audio_dma.h
  * @brief   Transmit DMA setup shared by the audio backends
  * @details The stream, channel and NVIC line are supplied by the active
  *          backend's operations table, so this module never needs to know
  *          whether it is feeding SAI or I2S.
  ******************************************************************************
  */

#ifndef AUDIO_DMA_H
#define AUDIO_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_types.h"

/**
 * @brief   Configure and start the transmit DMA for the active backend
 * @details Enables the controller clock, initialises the stream in circular
 *          memory-to-peripheral mode, hands the handle to the backend for
 *          linking and unmasks the stream interrupt.
 * @param   dev Device whose ops table selects the stream
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef Audio_DmaInit(AudioDevice_t* dev);

/**
 * @brief   Mask the stream interrupt and release the DMA stream
 * @param   dev Device to tear down
 * @retval  None
 */
void Audio_DmaDeInit(AudioDevice_t* dev);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_DMA_H */
