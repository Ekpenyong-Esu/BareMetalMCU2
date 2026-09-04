/**
 ******************************************************************************
 * @file    audio_buffer.h
 * @brief   Playback ring buffer for the audio subsystem
 * @details Owns the producer/consumer bookkeeping of the output buffer. Nothing
 *          in this module touches hardware, which makes the index arithmetic
 *          testable on its own.
 ******************************************************************************
 */

#ifndef AUDIO_BUFFER_H
#define AUDIO_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_types.h"

/**
 * @brief   Bind the handle's playback memory to its output buffer
 * @param   dev Device whose configuration selects the buffer size
 * @retval  AUDIO_StatusTypeDef AUDIO_OK, or AUDIO_ERROR if the requested size
 *          exceeds the memory reserved in the handle
 */
AUDIO_StatusTypeDef Audio_BufferInit(AUDIO_Handle_t *dev);

/**
 * @brief   Detach the buffer memory from the device
 * @param   dev Device to detach
 * @retval  None
 */
void Audio_BufferRelease(AUDIO_Handle_t *dev);

/**
 * @brief   Discard all buffered audio and rewind both indices
 * @param   dev Device to reset
 * @retval  None
 */
void Audio_BufferReset(AUDIO_Handle_t *dev);

/**
 * @brief   Number of bytes that can be written without overwriting unread data
 * @param   dev Device to query
 * @retval  uint32_t Free space in bytes
 */
uint32_t Audio_BufferFreeSpace(const AUDIO_Handle_t *dev);

/**
 * @brief   Append audio data to the playback buffer
 * @param   dev  Destination device
 * @param   data Source samples
 * @param   size Number of bytes to copy
 * @retval  AUDIO_StatusTypeDef AUDIO_OK, AUDIO_INVALID_PARAM or AUDIO_OVERFLOW
 */
AUDIO_StatusTypeDef Audio_BufferWrite(AUDIO_Handle_t *dev, const uint8_t *data, uint32_t size);

/**
 * @brief   Release one half-buffer back to the producer
 * @details Called from the transfer-complete callback once the DMA has finished
 *          streaming half of the circular buffer.
 * @param   dev Device to advance
 * @retval  None
 */
void Audio_BufferAdvanceRead(AUDIO_Handle_t *dev);

/**
 * @brief   Size of one audio frame in bytes for the device configuration
 * @param   dev Device to inspect
 * @retval  uint32_t Bytes per frame (all channels of one sample)
 */
uint32_t Audio_BufferFrameSize(const AUDIO_Handle_t *dev);

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_BUFFER_H */
