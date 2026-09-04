/**
 * @file    audio_core.h
 * @brief   Main audio control
 * @details This file starts and controls audio play and record.
 *          It uses I2S or SAI to send sound data. Use this file
 *          to init, play, pause, and stop sound.
 *
 *          The application owns the AUDIO_Handle_t and fills an
 *          AUDIO_ConfigTypeDef with the transport instance, its pins, the
 *          DMA stream and (optionally) the open I2C bus and reset line of
 *          the codec. Nothing in this driver assumes a board.
 */

#ifndef AUDIO_CORE_H
#define AUDIO_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_types.h"

/** @defgroup AUDIO_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize an audio handle from the caller's wiring and format
 * @details Configures pins, clocks, the transport and the DMA stream named in
 *          @p config. The codec is reset and programmed over config->codecBus
 *          only when that bus is not NULL.
 * @param   dev Caller-owned handle to initialise
 * @param   config Wiring and stream format
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Init(AUDIO_Handle_t *dev, const AUDIO_ConfigTypeDef *config);

/**
 * @brief   Deinitialize an audio handle
 * @details Stops playback and releases the transport, DMA and buffer.
 * @param   dev Handle to release
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_DeInit(AUDIO_Handle_t *dev);

/**
 * @brief   Reset and configure the audio codec again
 * @param   dev Handle whose config names the codec bus
 * @retval  AUDIO_StatusTypeDef AUDIO_NOT_READY when no codec bus was given
 */
AUDIO_StatusTypeDef AUDIO_CodecInit(AUDIO_Handle_t *dev);

/** @} */

/** @defgroup AUDIO_Data_Operations Data Operations
 * @{
 */

/**
 * @brief   Start audio playback from the output buffer
 * @param   dev Handle to play on
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Play(AUDIO_Handle_t *dev);

/**
 * @brief   Stop audio playback and flush the output buffer
 * @param   dev Handle to stop
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Stop(AUDIO_Handle_t *dev);

/**
 * @brief   Pause audio playback without flushing the output buffer
 * @param   dev Handle to pause
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Pause(AUDIO_Handle_t *dev);

/**
 * @brief   Resume audio playback from the paused state
 * @param   dev Handle to resume
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Resume(AUDIO_Handle_t *dev);

/**
 * @brief   Write audio data to the playback buffer
 * @param   dev Destination handle
 * @param   data Pointer to audio data
 * @param   size Size of data in bytes
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_WriteBuffer(AUDIO_Handle_t *dev, const uint8_t *data, uint32_t size);

/** @} */

/** @defgroup AUDIO_Control_Functions Control Functions
 * @{
 */

/**
 * @brief   Set audio output volume
 * @param   dev Handle whose codec to program
 * @param   volume Volume level (0-100)
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_SetVolume(AUDIO_Handle_t *dev, uint8_t volume);

/**
 * @brief   Get the current audio volume
 * @param   dev Handle to query
 * @param   volume Destination for the volume level
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetVolume(const AUDIO_Handle_t *dev, uint8_t *volume);

/**
 * @brief   Mute or unmute the audio output
 * @param   dev Handle whose codec to program
 * @param   mute True to mute, false to unmute
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_SetMute(AUDIO_Handle_t *dev, bool mute);

/**
 * @brief   Get the current mute state
 * @param   dev Handle to query
 * @param   mute Destination for the mute state
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetMute(const AUDIO_Handle_t *dev, bool *mute);

/** @} */

/** @defgroup AUDIO_Monitoring_Functions Monitoring and Statistics
 * @{
 */

/**
 * @brief   Get the audio subsystem status
 * @param   dev Handle to query
 * @retval  AUDIO_StatusTypeDef AUDIO_ERROR after a transfer fault, else AUDIO_OK
 */
AUDIO_StatusTypeDef AUDIO_GetStatus(const AUDIO_Handle_t *dev);

/**
 * @brief   Get audio runtime statistics
 * @param   dev Handle to query
 * @param   stats Destination statistics structure
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetStatistics(const AUDIO_Handle_t *dev, AUDIO_StatsTypeDef *stats);

/**
 * @brief   Convert a status code to a human-readable string
 * @param   status Audio status code
 * @retval  const char* Status description
 */
const char *AUDIO_GetStatusString(AUDIO_StatusTypeDef status);

/** @} */

/** @defgroup AUDIO_Internal_Access Internal Access
 * @{
 */

/** Handles the driver can service at once: I2S2, I2S3, SAI1_A, SAI1_B. */
#define AUDIO_MAX_HANDLES 4U

/**
 * @brief   Resolve the handle that owns a HAL SAI handle
 * @details Used by audio_events.c to route HAL callbacks; the HAL handle has
 *          no parent pointer, so the registry filled by AUDIO_Init is searched.
 * @retval  AUDIO_Handle_t* Owning handle, or NULL when unregistered
 */
AUDIO_Handle_t *AUDIO_FromSai(const SAI_HandleTypeDef *hsai);

/**
 * @brief   Resolve the handle that owns a HAL I2S handle
 * @retval  AUDIO_Handle_t* Owning handle, or NULL when unregistered
 */
AUDIO_Handle_t *AUDIO_FromI2s(const I2S_HandleTypeDef *hi2s);

/**
 * @brief   Registered handle by slot, for the interrupt entry point
 * @param   slot 0..AUDIO_MAX_HANDLES-1
 * @retval  AUDIO_Handle_t* Handle in that slot, or NULL when free
 */
AUDIO_Handle_t *AUDIO_HandleAt(uint32_t slot);

/**
 * @brief   Transmit DMA interrupt entry point
 * @details Core owns the vector table and must call this from the DMA stream
 *          handler of every stream named in an AUDIO_ConfigTypeDef. Every
 *          registered handle is serviced; HAL ignores streams with nothing
 *          pending.
 * @retval  None
 */
void AUDIO_IRQHandler(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_CORE_H */
