/**
  ******************************************************************************
  * @file    audio_types.h
  * @brief   Shared vocabulary for the audio subsystem
  * @details Declares the enums, configuration/statistics structures, the audio
  *          interface operations table and the device record that every audio
  *          module operates on. This header contains no behaviour; it exists so
  *          that the backends (SAI/I2S), the codec, the ring buffer and the core
  *          all speak the same language without depending on each other.
  ******************************************************************************
  */

#ifndef AUDIO_TYPES_H
#define AUDIO_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_sai.h"
#include "stm32f4xx_hal_i2s.h"
#include "stm32f4xx_hal_dma.h"
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
    AUDIO_CHANNEL_MONO = 0,     /**< Mono audio */
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
 * @brief Audio driver lifecycle state
 */
typedef enum {
    AUDIO_STATE_RESET = 0,      /**< Not initialised */
    AUDIO_STATE_READY,          /**< Initialised, not streaming */
    AUDIO_STATE_PLAYING,        /**< Streaming to the codec */
    AUDIO_STATE_PAUSED,         /**< Stream suspended, buffers retained */
    AUDIO_STATE_ERROR           /**< Unrecoverable transfer error */
} AUDIO_StateTypeDef;

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
 * @brief Audio ring buffer structure
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

/** Forward declaration: the operations table receives the device it acts on. */
typedef struct AudioDevice AudioDevice_t;

/**
 * @brief Audio interface backend operations
 * @details One instance of this table exists per transport (SAI, I2S). The core
 *          resolves the table once during initialisation, which is why no other
 *          module ever has to branch on AUDIO_InterfaceTypeDef.
 */
typedef struct {
    /** Human readable backend name, for diagnostics. */
    const char* name;

    /** DMA stream that carries this backend's transmit data. */
    DMA_Stream_TypeDef* dmaStream;
    /** DMA channel selection for @ref dmaStream. */
    uint32_t dmaChannel;
    /** NVIC line raised by @ref dmaStream. */
    IRQn_Type dmaIrq;

    /** Configure clocks, pins and the transport peripheral. */
    AUDIO_StatusTypeDef (*init)(AudioDevice_t* dev);
    /** Release the transport peripheral and its clock. */
    void (*deinit)(AudioDevice_t* dev);
    /** Attach the already initialised DMA handle to the transport. */
    void (*linkDma)(AudioDevice_t* dev);
    /** Begin a circular DMA transmission of the output buffer. */
    AUDIO_StatusTypeDef (*start)(AudioDevice_t* dev);
    /** Abort the transmission. */
    void (*stop)(AudioDevice_t* dev);
    /** Suspend the transmission, keeping the DMA state. */
    void (*pause)(AudioDevice_t* dev);
    /** Continue a suspended transmission. */
    void (*resume)(AudioDevice_t* dev);
} AudioIfOps_t;

/**
 * @brief Audio device record
 * @details All mutable driver state lives here instead of in scattered file
 *          scope variables, so the whole subsystem can be inspected, reset or
 *          (in future) duplicated by touching a single object.
 */
struct AudioDevice {
    const AudioIfOps_t*   ops;      /**< Resolved transport backend */
    AUDIO_ConfigTypeDef   config;   /**< Active configuration */
    AUDIO_StateTypeDef    state;    /**< Lifecycle state */
    AUDIO_BufferTypeDef   output;   /**< Playback ring buffer */
    AUDIO_StatsTypeDef    stats;    /**< Runtime counters */

    SAI_HandleTypeDef     sai;      /**< HAL handle used when Interface == SAI */
    I2S_HandleTypeDef     i2s;      /**< HAL handle used when Interface == I2S */
    DMA_HandleTypeDef     dma;      /**< Transmit DMA handle */

    uint8_t               volume;   /**< Cached volume, 0..AUDIO_VOLUME_MAX */
    bool                  muted;    /**< Cached mute state */
};

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
#define AUDIO_SAMPLE_MAX_16BIT       32767      /**< Maximum 16-bit sample value */
#define AUDIO_SAMPLE_MAX_24BIT       8388607    /**< Maximum 24-bit sample value */
#define AUDIO_SAMPLE_MAX_32BIT       2147483647 /**< Maximum 32-bit sample value */

/** @} */

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Translate a configured sample rate to hertz
 * @param   rate Configured sample rate
 * @retval  uint32_t Sample rate in Hz
 */
uint32_t Audio_SampleRateHz(AUDIO_FreqTypeDef rate);

/**
 * @brief   Guard used by every entry point that requires a live device
 * @param   dev Device record to validate
 * @retval  AUDIO_OK when the device is initialised, otherwise the reason why not
 */
static inline AUDIO_StatusTypeDef Audio_CheckReady(const AudioDevice_t* dev)
{
    if (dev == NULL || dev->ops == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    if (dev->state == AUDIO_STATE_RESET) {
        return AUDIO_NOT_READY;
    }
    return AUDIO_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_TYPES_H */
