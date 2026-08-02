/**
  ******************************************************************************
  * @file    audio.h
  * @brief   Audio subsystem interface for STM32F429
  * @details This file contains function prototypes and definitions for
  *          audio processing, codec control, and audio I/O operations.
  * @version 1.0
  * @date    2025-09-01
  ******************************************************************************
  */

#ifndef AUDIO_H
#define AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Audio status enumeration
 */
typedef enum {
    AUDIO_OK = 0,           /**< Operation completed successfully */
    AUDIO_ERROR,            /**< General error occurred */
    AUDIO_BUSY,             /**< Audio peripheral is busy */
    AUDIO_TIMEOUT,          /**< Operation timed out */
    AUDIO_INVALID_PARAM,    /**< Invalid parameter provided */
    AUDIO_NOT_READY,        /**< Audio system not ready */
    AUDIO_OVERFLOW,         /**< Audio buffer overflow */
    AUDIO_UNDERFLOW         /**< Audio buffer underflow */
} AUDIO_StatusTypeDef;

/**
 * @brief Audio format enumeration
 */
typedef enum {
    AUDIO_FORMAT_16BIT = 0, /**< 16-bit audio */
    AUDIO_FORMAT_24BIT,     /**< 24-bit audio */
    AUDIO_FORMAT_32BIT      /**< 32-bit audio */
} AUDIO_FormatTypeDef;

/**
 * @brief Audio sample rate enumeration
 */
typedef enum {
    AUDIO_FREQ_8K = 0,      /**< 8 kHz */
    AUDIO_FREQ_11K,         /**< 11.025 kHz */
    AUDIO_FREQ_16K,         /**< 16 kHz */
    AUDIO_FREQ_22K,         /**< 22.05 kHz */
    AUDIO_FREQ_32K,         /**< 32 kHz */
    AUDIO_FREQ_44K,         /**< 44.1 kHz */
    AUDIO_FREQ_48K,         /**< 48 kHz */
    AUDIO_FREQ_96K          /**< 96 kHz */
} AUDIO_FreqTypeDef;

/**
 * @brief Audio channel configuration
 */
typedef enum {
    AUDIO_CHANNEL_MONO = 0,    /**< Mono audio */
    AUDIO_CHANNEL_STEREO        /**< Stereo audio */
} AUDIO_ChannelTypeDef;

/**
 * @brief Audio interface type
 */
typedef enum {
    AUDIO_INTERFACE_SAI = 0,    /**< Serial Audio Interface */
    AUDIO_INTERFACE_I2S         /**< Inter-IC Sound */
} AUDIO_InterfaceTypeDef;

/**
 * @brief Audio configuration structure
 */
typedef struct {
    AUDIO_InterfaceTypeDef Interface;    /**< Audio interface type */
    AUDIO_FreqTypeDef SampleRate;        /**< Audio sample rate */
    AUDIO_FormatTypeDef BitDepth;        /**< Audio bit depth */
    AUDIO_ChannelTypeDef Channels;       /**< Audio channels */
    uint32_t BufferSize;                 /**< Audio buffer size */
    bool EnableDMA;                      /**< Enable DMA transfers */
} AUDIO_ConfigTypeDef;

/**
 * @brief Audio buffer structure
 */
typedef struct {
    uint8_t* Buffer;          /**< Audio data buffer */
    uint32_t Size;            /**< Buffer size in bytes */
    uint32_t ReadIndex;       /**< Read index for circular buffer */
    uint32_t WriteIndex;      /**< Write index for circular buffer */
    bool IsFull;              /**< Buffer full flag */
    bool IsEmpty;             /**< Buffer empty flag */
} AUDIO_BufferTypeDef;

/**
 * @brief Audio statistics structure
 */
typedef struct {
    uint32_t SampleCount;     /**< Total samples played */
    uint32_t BufferOverflows; /**< Buffer overflow count */
    uint32_t SyncErrors;      /**< Synchronization error count */
} AUDIO_StatsTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup AUDIO_Constants Audio Driver Constants
 * @{
 */

#define AUDIO_BUFFER_SIZE_DEFAULT    4096U   /**< Default buffer size */

/**
 * @brief Audio volume levels
 */
#define AUDIO_VOLUME_MAX             100U    /**< Maximum volume */
#define AUDIO_VOLUME_DEFAULT         75U     /**< Default volume */

/**
 * @brief Audio processing constants
 */
#define AUDIO_SAMPLE_MAX_16BIT       32767   /**< Maximum 16-bit sample value */
#define AUDIO_SAMPLE_MAX_24BIT       8388607 /**< Maximum 24-bit sample value */
#define AUDIO_SAMPLE_MAX_32BIT       2147483647 /**< Maximum 32-bit sample value */

/** @} */

/* Exported functions prototypes ---------------------------------------------*/

/** @defgroup AUDIO_Init_Config Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize the audio subsystem
 * @details Configures audio peripherals and initializes the audio system
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Init(void);

/**
 * @brief   Initialize audio with custom configuration
 * @details Allows custom configuration of audio parameters
 * @param   config Pointer to audio configuration structure
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Init_Custom(const AUDIO_ConfigTypeDef* config);

/**
 * @brief   Deinitialize the audio subsystem
 * @details Stops audio processing and releases resources
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_DeInit(void);

/** @} */

/** @defgroup AUDIO_Data_Operations Data Operations
 * @{
 */

/**
 * @brief   Start audio playback
 * @details Begins audio output from the configured buffer
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Play(void);

/**
 * @brief   Stop audio playback
 * @details Stops audio output and flushes buffers
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Stop(void);

/**
 * @brief   Pause audio playback
 * @details Pauses audio output without flushing buffers
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Pause(void);

/**
 * @brief   Resume audio playback
 * @details Resumes audio output from paused state
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Resume(void);

/** @defgroup AUDIO_Buffer_Management Buffer Management
 * @{
 */

/**
 * @brief   Write audio data to output buffer
 * @details Adds audio samples to the playback buffer
 * @param   data Pointer to audio data buffer
 * @param   size Size of data in bytes
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_WriteBuffer(uint8_t* data, uint32_t size);

/** @} */

/** @defgroup AUDIO_Control_Functions Control Functions
 * @{
 */

/**
 * @brief   Set audio volume
 * @details Adjusts the audio output volume level
 * @param   volume Volume level (0-100)
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_SetVolume(uint8_t volume);

/**
 * @brief   Get current audio volume
 * @details Returns the current volume level
 * @param   volume Pointer to store volume level
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetVolume(uint8_t* volume);

/**
 * @brief   Set audio mute state
 * @details Mutes or unmutes audio output
 * @param   mute True to mute, false to unmute
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_SetMute(bool mute);

/**
 * @brief   Get audio mute state
 * @details Returns the current mute state
 * @param   mute Pointer to store mute state
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetMute(bool* mute);

/** @} */

/** @defgroup AUDIO_Codec_Functions Codec Control
 * @{
 */

/**
 * @brief   Initialize audio codec
 * @details Configures the connected audio codec
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_CodecInit(void);

/** @} */

/** @defgroup AUDIO_Monitoring_Functions Monitoring and Statistics
 * @{
 */

/**
 * @brief   Get audio system status
 * @details Returns current audio system state
 * @param   None
 * @retval  AUDIO_StatusTypeDef Current system status
 */
AUDIO_StatusTypeDef AUDIO_GetStatus(void);

/**
 * @brief   Get audio statistics
 * @details Returns performance and error statistics
 * @param   stats Pointer to statistics structure
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetStatistics(AUDIO_StatsTypeDef* stats);

/** @} */

/** @defgroup AUDIO_Utility_Functions Utility Functions
 * @{
 */

/**
 * @brief   Get audio status string
 * @details Converts status code to human-readable string
 * @param   status Audio status code
 * @retval  const char* Status description string
 */
const char* AUDIO_GetStatusString(AUDIO_StatusTypeDef status);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_H */
