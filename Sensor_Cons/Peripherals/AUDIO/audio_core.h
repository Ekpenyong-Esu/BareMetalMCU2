/**
  * @file    audio_core.h
  * @brief   Main audio control
  * @details This file starts and controls audio play and record.
  *          It uses I2S or SAI to send sound data. Use this file
  *          to init, play, pause, and stop sound.
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
 * @brief   Initialize the audio subsystem with the default configuration
 * @details 44.1 kHz, 16-bit stereo over SAI with DMA enabled.
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Init(void);

/**
 * @brief   Initialize audio with a custom configuration
 * @param   config Pointer to audio configuration structure
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Init_Custom(const AUDIO_ConfigTypeDef* config);

/**
 * @brief   Deinitialize the audio subsystem
 * @details Stops playback and releases the transport, DMA and buffer.
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_DeInit(void);

/**
 * @brief   Reset and configure the audio codec
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_CodecInit(void);

/** @} */

/** @defgroup AUDIO_Data_Operations Data Operations
 * @{
 */

/**
 * @brief   Start audio playback from the output buffer
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Play(void);

/**
 * @brief   Stop audio playback and flush the output buffer
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Stop(void);

/**
 * @brief   Pause audio playback without flushing the output buffer
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Pause(void);

/**
 * @brief   Resume audio playback from the paused state
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Resume(void);

/**
 * @brief   Write audio data to the playback buffer
 * @param   data Pointer to audio data
 * @param   size Size of data in bytes
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_WriteBuffer(const uint8_t* data, uint32_t size);

/** @} */

/** @defgroup AUDIO_Control_Functions Control Functions
 * @{
 */

/**
 * @brief   Set audio output volume
 * @param   volume Volume level (0-100)
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_SetVolume(uint8_t volume);

/**
 * @brief   Get the current audio volume
 * @param   volume Destination for the volume level
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetVolume(uint8_t* volume);

/**
 * @brief   Mute or unmute the audio output
 * @param   mute True to mute, false to unmute
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_SetMute(bool mute);

/**
 * @brief   Get the current mute state
 * @param   mute Destination for the mute state
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetMute(bool* mute);

/** @} */

/** @defgroup AUDIO_Monitoring_Functions Monitoring and Statistics
 * @{
 */

/**
 * @brief   Get the audio subsystem status
 * @retval  AUDIO_StatusTypeDef AUDIO_ERROR after a transfer fault, else AUDIO_OK
 */
AUDIO_StatusTypeDef AUDIO_GetStatus(void);

/**
 * @brief   Get audio runtime statistics
 * @param   stats Destination statistics structure
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetStatistics(AUDIO_StatsTypeDef* stats);

/**
 * @brief   Convert a status code to a human-readable string
 * @param   status Audio status code
 * @retval  const char* Status description
 */
const char* AUDIO_GetStatusString(AUDIO_StatusTypeDef status);

/** @} */

/** @defgroup AUDIO_Internal_Access Internal Access
 * @{
 */

/**
 * @brief   Access the single audio device record
 * @details Used by audio_events.c to match HAL callbacks against the handles
 *          this driver owns. Application code should prefer the API above.
 * @retval  AudioDevice_t* The device record; never NULL
 */
AudioDevice_t* AUDIO_Device(void);

/**
 * @brief   Transmit DMA interrupt entry point
 * @details Core owns the vector table and must call this from the DMA stream
 *          handler selected by the active backend.
 * @retval  None
 */
void AUDIO_IRQHandler(void);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_CORE_H */
