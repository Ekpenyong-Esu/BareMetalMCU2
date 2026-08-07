/**
  ******************************************************************************
  * @file    mic_types.h
  * @brief   Shared vocabulary for the MP45DT02 PDM microphone driver
  ******************************************************************************
  */

#ifndef MIC_TYPES_H
#define MIC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "stm32f4xx_hal_i2s.h"
#include "stm32f4xx_hal_dma.h"
#include <stdint.h>
#include <stdbool.h>

/* Limits and defaults -------------------------------------------------------*/
#define MIC_OPERATION_TIMEOUT           1000
#define MIC_MAX_SAMPLES                 8192
#define MIC_DEFAULT_SAMPLE_RATE         16000   /* Hz */
#define MIC_DEFAULT_VOLUME              75      /* Percentage */
#define MIC_MAX_VOLUME                  100

/* MP45DT02 datasheet figures ------------------------------------------------*/
#define MIC_SENSITIVITY                 (-26.0f)  /* dBFS */
#define MIC_SNR                         (61.0f)   /* dB */
#define MIC_AOP                         (120.0f)  /* dB SPL */
#define MIC_FREQUENCY_RESPONSE_MIN      100       /* Hz */
#define MIC_FREQUENCY_RESPONSE_MAX      10000     /* Hz */

/* Board wiring --------------------------------------------------------------*/
/* MP45DT02 PDM microphone sits on I2S2; I2S3/SPI3 belongs to the audio codec. */
#define MIC_CLK_PIN                     GPIO_PIN_10   /* PB10 - I2S2_CK */
#define MIC_CLK_GPIO_PORT               GPIOB
#define MIC_DATA_PIN                    GPIO_PIN_3    /* PC3 - I2S2_SD (PDM_OUT) */
#define MIC_DATA_GPIO_PORT              GPIOC
#define MIC_DATA_EXTI_IRQn              EXTI3_IRQn

#define MIC_I2S                         SPI2
#define MIC_I2S_IRQ                     SPI2_IRQn
#define MIC_I2S_CLK_ENABLE()            __HAL_RCC_SPI2_CLK_ENABLE()
#define MIC_I2S_CLK_DISABLE()           __HAL_RCC_SPI2_CLK_DISABLE()

/* SPI2_RX is only mapped to DMA1 Stream3 Channel 0. */
#define MIC_DMA_STREAM                  DMA1_Stream3
#define MIC_DMA_CHANNEL                 DMA_CHANNEL_0
#define MIC_DMA_IRQ                     DMA1_Stream3_IRQn
#define MIC_DMA_CLK_ENABLE()            __HAL_RCC_DMA1_CLK_ENABLE()

/* Signal chain --------------------------------------------------------------*/
#define MIC_PCM_SAMPLES                 128
#define MIC_PDM_BUFFER_SIZE             512
#define MIC_DECIMATION_FACTOR           64
#define MIC_PDM_WORD_SIZE               32      /* PDM word size in bits */
#define MIC_PCM_MAX_VALUE               32767
#define MIC_PCM_MIN_VALUE               (-32768)
#define MIC_PCM_SCALE_FACTOR            32768.0f
#define MIC_FFT_SIZE                    64
#define MIC_SPECTRAL_BINS               32

#define MIC_GAIN_DEFAULT                (1.0f)
#define MIC_HP_FILTER_COEFF             (0.95f)
#define MIC_VOLUME_SCALE_FACTOR         (0.01f)
#define MIC_STATISTICS_ALPHA            (0.1f)
#define MIC_NOISE_FLOOR_LINEAR          (0.001f)  /* -60 dB */
#define MIC_NOISE_FLOOR                 (-60.0f)  /* dB */
#define MIC_DB_SCALE_FACTOR             (20.0f)
#define MIC_LOG_BASE                    (10.0f)
#define MIC_FFT_SCALE_FACTOR            (2.0f)

/* Exported types ------------------------------------------------------------*/

typedef enum {
    MIC_OK = 0,                     /**< Operation completed successfully */
    MIC_ERROR,                      /**< General error occurred */
    MIC_BUSY,                       /**< Microphone is busy */
    MIC_TIMEOUT,                    /**< Operation timed out */
    MIC_INVALID_PARAM,              /**< Invalid parameter provided */
    MIC_NOT_INITIALIZED,            /**< Device not initialized */
    MIC_BUFFER_OVERFLOW,            /**< Buffer overflow occurred */
    MIC_DMA_ERROR,                  /**< DMA transfer error */
    MIC_I2S_ERROR                   /**< I2S communication error */
} MIC_StatusTypeDef;

typedef enum {
    MIC_SAMPLE_RATE_8KHZ = 8000,
    MIC_SAMPLE_RATE_16KHZ = 16000,
    MIC_SAMPLE_RATE_22KHZ = 22050,
    MIC_SAMPLE_RATE_32KHZ = 32000,
    MIC_SAMPLE_RATE_44KHZ = 44100,
    MIC_SAMPLE_RATE_48KHZ = 48000
} MIC_SampleRateTypeDef;

typedef enum {
    MIC_GAIN_0DB = 0,
    MIC_GAIN_6DB,
    MIC_GAIN_12DB,
    MIC_GAIN_18DB,
    MIC_GAIN_24DB
} MIC_GainTypeDef;

typedef struct {
    MIC_SampleRateTypeDef SampleRate;       /**< Audio sample rate */
    MIC_GainTypeDef Gain;                   /**< Microphone gain */
    uint8_t Volume;                         /**< Volume level (0-100) */
    bool NoiseGateEnable;                   /**< Noise gate enable */
    float NoiseGateThreshold;               /**< Noise gate threshold in dB */
    bool HighPassFilter;                    /**< High-pass filter enable */
    uint16_t BufferSize;                    /**< Audio buffer size in samples */
} MIC_ConfigTypeDef;

typedef struct {
    uint32_t Size;                          /**< Buffer size in samples */
    uint32_t Position;                      /**< Current position */
    bool IsFull;                            /**< Buffer full flag */
    uint32_t Timestamp;                     /**< Buffer timestamp */
} MIC_AudioBufferTypeDef;

typedef struct {
    float RMSLevel;                         /**< RMS audio level */
    float PeakLevel;                        /**< Peak audio level */
    float SNR;                              /**< Signal-to-noise ratio */
    uint32_t SampleCount;                   /**< Total samples processed */
    uint32_t OverrunCount;                  /**< Buffer overrun count */
    uint32_t DropoutCount;                  /**< Audio dropout count */
    float AverageLevel;                     /**< Average audio level */
} MIC_StatisticsTypeDef;

typedef struct {
    I2S_HandleTypeDef *hi2s;                /**< I2S handle */
    DMA_HandleTypeDef *hdma;                /**< DMA handle */
    MIC_ConfigTypeDef Config;               /**< Microphone configuration */
    MIC_AudioBufferTypeDef AudioBuffer;     /**< Capture bookkeeping */
    MIC_StatisticsTypeDef Statistics;       /**< Audio statistics */
    bool IsInitialized;                     /**< Initialization status */
    bool IsRecording;                       /**< Recording status */
    bool IsPaused;                          /**< Pause status */
    uint32_t PDMBuffer[MIC_PDM_BUFFER_SIZE];/**< PDM data buffer */
    int16_t PCMBuffer[MIC_PCM_SAMPLES];     /**< PCM output buffer */
    volatile bool BufferReady;              /**< Buffer ready flag */
    int16_t HpPrevInput;                    /**< High-pass filter previous input */
    int16_t HpPrevOutput;                   /**< High-pass filter previous output */
    void (*RecordCallback)(void);           /**< Record complete callback */
    void (*ErrorCallback)(void);            /**< Error callback */
} MIC_HandleTypeDef;

typedef struct {
    float FrequencyBins[MIC_FFT_SIZE];      /**< Frequency domain bins */
    float MagnitudeSpectrum[MIC_FFT_SIZE];  /**< Magnitude spectrum */
    float PhaseSpectrum[MIC_FFT_SIZE];      /**< Phase spectrum */
    float Centroid;                         /**< Spectral centroid */
    float Bandwidth;                        /**< Spectral bandwidth */
    float ZeroCrossingRate;                 /**< Zero crossing rate */
} MIC_AudioAnalysisTypeDef;

/**
 * @brief Shared guard: handle present and brought up.
 */
static inline MIC_StatusTypeDef MIC_CheckReady(const MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }
    if (!hmic->IsInitialized) {
        return MIC_NOT_INITIALIZED;
    }
    return MIC_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* MIC_TYPES_H */
