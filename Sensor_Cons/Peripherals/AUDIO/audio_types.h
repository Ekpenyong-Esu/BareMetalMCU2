/**
 * @file    audio_types.h
 * @brief   Shared types for the audio system
 * @details This file holds the common types for playing and recording
 *          sound. Sound goes through I2S or SAI. These are fast buses
 *          made for audio. This file has the settings and status codes
 *          used by the audio driver.
 *
 *          The application owns one AUDIO_Handle_t per codec and says in
 *          AUDIO_ConfigTypeDef which transport instance, pins, DMA stream
 *          and control bus carry it. The driver never picks hardware.
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
#include "i2c_types.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup AUDIO_Constants Audio Driver Constants
 * @{
 */

#define AUDIO_BUFFER_SIZE_DEFAULT 4096U /**< Default and maximum buffer size */

/**
 * @brief Audio volume levels
 */
#define AUDIO_VOLUME_MAX 100U    /**< Maximum volume */
#define AUDIO_VOLUME_DEFAULT 75U /**< Default volume */

/**
 * @brief Audio processing constants
 */
#define AUDIO_SAMPLE_MAX_16BIT 32767      /**< Maximum 16-bit sample value */
#define AUDIO_SAMPLE_MAX_24BIT 8388607    /**< Maximum 24-bit sample value */
#define AUDIO_SAMPLE_MAX_32BIT 2147483647 /**< Maximum 32-bit sample value */

/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Audio status enumeration
 */
typedef enum {
    AUDIO_OK = 0,        /**< Operation completed successfully */
    AUDIO_ERROR,         /**< General error occurred */
    AUDIO_BUSY,          /**< Audio peripheral is busy */
    AUDIO_TIMEOUT,       /**< Operation timed out */
    AUDIO_INVALID_PARAM, /**< Invalid parameter provided */
    AUDIO_NOT_READY,     /**< Audio system not ready */
    AUDIO_OVERFLOW,      /**< Audio buffer overflow */
    AUDIO_UNDERFLOW      /**< Audio buffer underflow */
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
    AUDIO_FREQ_8K = 0, /**< 8 kHz */
    AUDIO_FREQ_11K,    /**< 11.025 kHz */
    AUDIO_FREQ_16K,    /**< 16 kHz */
    AUDIO_FREQ_22K,    /**< 22.05 kHz */
    AUDIO_FREQ_32K,    /**< 32 kHz */
    AUDIO_FREQ_44K,    /**< 44.1 kHz */
    AUDIO_FREQ_48K,    /**< 48 kHz */
    AUDIO_FREQ_96K     /**< 96 kHz */
} AUDIO_FreqTypeDef;

/**
 * @brief Audio channel configuration
 */
typedef enum {
    AUDIO_CHANNEL_MONO = 0, /**< Mono audio */
    AUDIO_CHANNEL_STEREO    /**< Stereo audio */
} AUDIO_ChannelTypeDef;

/**
 * @brief Audio interface type
 */
typedef enum {
    AUDIO_INTERFACE_SAI = 0, /**< Serial Audio Interface */
    AUDIO_INTERFACE_I2S      /**< Inter-IC Sound */
} AUDIO_InterfaceTypeDef;

/**
 * @brief Audio driver lifecycle state
 */
typedef enum {
    AUDIO_STATE_RESET = 0, /**< Not initialised */
    AUDIO_STATE_READY,     /**< Initialised, not streaming */
    AUDIO_STATE_PLAYING,   /**< Streaming to the codec */
    AUDIO_STATE_PAUSED,    /**< Stream suspended, buffers retained */
    AUDIO_STATE_ERROR      /**< Unrecoverable transfer error */
} AUDIO_StateTypeDef;

/**
 * @brief One GPIO line; a NULL port means the signal is not wired
 */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} AUDIO_Pin_t;

/**
 * @brief Audio configuration structure
 * @details Format fields describe the stream. The binding fields describe
 *          the board: only the instance matching @ref Interface is read,
 *          the other may stay NULL.
 */
typedef struct {
    AUDIO_InterfaceTypeDef Interface; /**< Audio interface type */
    AUDIO_FreqTypeDef SampleRate;     /**< Audio sample rate */
    AUDIO_FormatTypeDef BitDepth;     /**< Audio bit depth */
    AUDIO_ChannelTypeDef Channels;    /**< Audio channels */
    uint32_t BufferSize;              /**< Audio buffer size, at most AUDIO_BUFFER_SIZE_DEFAULT */
    bool EnableDMA;                   /**< Enable DMA transfers */

    /* Transport binding */
    SPI_TypeDef *i2sInstance;      /**< SPI2 or SPI3 when Interface == I2S */
    SAI_Block_TypeDef *saiBlock;   /**< SAI1_Block_A or _B when Interface == SAI */
    AUDIO_Pin_t wsPin;             /**< Word select (I2S WS / SAI FS) */
    AUDIO_Pin_t ckPin;             /**< Bit clock (I2S CK / SAI SCK) */
    AUDIO_Pin_t sdPin;             /**< Serial data */
    AUDIO_Pin_t mclkPin;           /**< Master clock; not wired disables MCLK output */
    uint8_t alternate;             /**< GPIO_AFx, or 0 to derive it from the instance */
    DMA_Stream_TypeDef *dmaStream; /**< Transmit stream, required when EnableDMA */
    uint32_t dmaChannel;           /**< DMA_CHANNEL_x for @ref dmaStream */

    /* Codec binding */
    I2C_Bus_t *codecBus;       /**< Open control bus; NULL means transport only */
    uint16_t codecAddress;     /**< 8-bit write address, 0 for the CS43L22 default */
    AUDIO_Pin_t codecResetPin; /**< Codec RESET line; not wired skips the reset pulse */
} AUDIO_ConfigTypeDef;

/**
 * @brief Audio ring buffer structure
 */
typedef struct {
    uint8_t *Buffer;     /**< Audio data buffer */
    uint32_t Size;       /**< Buffer size in bytes */
    uint32_t ReadIndex;  /**< Read index for circular buffer */
    uint32_t WriteIndex; /**< Write index for circular buffer */
    bool IsFull;         /**< Buffer full flag */
    bool IsEmpty;        /**< Buffer empty flag */
} AUDIO_BufferTypeDef;

/**
 * @brief Audio statistics structure
 */
typedef struct {
    uint32_t SampleCount;     /**< Total samples played */
    uint32_t BufferOverflows; /**< Buffer overflow count */
    uint32_t SyncErrors;      /**< Synchronization error count */
} AUDIO_StatsTypeDef;

/** Forward declaration: the operations table receives the handle it acts on. */
typedef struct AUDIO_Handle AUDIO_Handle_t;

/**
 * @brief Audio interface backend operations
 * @details One instance of this table exists per transport (SAI, I2S). The core
 *          resolves the table once during initialisation, which is why no other
 *          module ever has to branch on AUDIO_InterfaceTypeDef.
 */
typedef struct {
    /** Human readable backend name, for diagnostics. */
    const char *name;

    /** Configure clocks, pins and the transport peripheral from the config. */
    AUDIO_StatusTypeDef (*init)(AUDIO_Handle_t *dev);
    /** Release the transport peripheral, its pins and its clock. */
    void (*deinit)(AUDIO_Handle_t *dev);
    /** Attach the already initialised DMA handle to the transport. */
    void (*linkDma)(AUDIO_Handle_t *dev);
    /** Begin a circular DMA transmission of the output buffer. */
    AUDIO_StatusTypeDef (*start)(AUDIO_Handle_t *dev);
    /** Abort the transmission. */
    void (*stop)(AUDIO_Handle_t *dev);
    /** Suspend the transmission, keeping the DMA state. */
    void (*pause)(AUDIO_Handle_t *dev);
    /** Continue a suspended transmission. */
    void (*resume)(AUDIO_Handle_t *dev);
} AudioIfOps_t;

/**
 * @brief Audio device handle, owned by the application
 * @details All mutable driver state lives here instead of in file scope, so
 *          two codecs on different transports are two handles, and the DMA
 *          memory travels with the handle it feeds.
 */
struct AUDIO_Handle {
    const AudioIfOps_t *ops;    /**< Resolved transport backend */
    AUDIO_ConfigTypeDef config; /**< Wiring and format this handle was opened with */
    AUDIO_StateTypeDef state;   /**< Lifecycle state */
    AUDIO_BufferTypeDef output; /**< Playback ring buffer */
    AUDIO_StatsTypeDef stats;   /**< Runtime counters */

    SAI_HandleTypeDef sai; /**< HAL handle used when Interface == SAI */
    I2S_HandleTypeDef i2s; /**< HAL handle used when Interface == I2S */
    DMA_HandleTypeDef dma; /**< Transmit DMA handle */
    IRQn_Type dmaIrq;      /**< NVIC line of config.dmaStream */
    I2C_Device_t codec;    /**< Codec registered on config.codecBus */

    uint8_t volume; /**< Cached volume, 0..AUDIO_VOLUME_MAX */
    bool muted;     /**< Cached mute state */

    /** Playback memory the DMA streams from; the driver never allocates. */
    uint8_t outputMemory[AUDIO_BUFFER_SIZE_DEFAULT];
};

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Translate a configured sample rate to hertz
 * @param   rate Configured sample rate
 * @retval  uint32_t Sample rate in Hz
 */
uint32_t Audio_SampleRateHz(AUDIO_FreqTypeDef rate);

/**
 * @brief   Whether a configured line is wired
 */
static inline bool Audio_PinIsWired(const AUDIO_Pin_t *pin) {
    return (pin->port != NULL) && (pin->pin != 0U);
}

/**
 * @brief   Guard used by every entry point that requires a live handle
 * @param   dev Handle to validate
 * @retval  AUDIO_OK when the handle is initialised, otherwise the reason why not
 */
static inline AUDIO_StatusTypeDef Audio_CheckReady(const AUDIO_Handle_t *dev) {
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
