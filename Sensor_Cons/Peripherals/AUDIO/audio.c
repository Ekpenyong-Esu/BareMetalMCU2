/**
  ******************************************************************************
  * @file    audio.c
  * @brief   Audio subsystem implementation for STM32F429
  * @details This file contains the implementation of audio processing functions,
  *          codec control, and audio I/O operations using SAI/I2S interfaces.
  * @version 1.0
  * @date    2025-09-01
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "audio.h"
#include "gpio.h"
#include "i2c.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_sai.h"
#include "stm32f4xx_hal_i2s.h"
#include "stm32f4xx_hal_dma.h"
#include <string.h>

/* Private defines -----------------------------------------------------------*/

/** @defgroup AUDIO_Private_Defines Private Defines
 * @{
 */

/**
 * @brief SAI peripheral configuration
 */
#define AUDIO_SAI_INSTANCE               SAI1_Block_A
#define AUDIO_SAI_CLK_ENABLE()           __HAL_RCC_SAI1_CLK_ENABLE()
#define AUDIO_SAI_CLK_DISABLE()          __HAL_RCC_SAI1_CLK_DISABLE()
#define AUDIO_SAI_MCK_PIN                GPIO_PIN_2
#define AUDIO_SAI_MCK_GPIO_PORT          GPIOE
#define AUDIO_SAI_SD_PIN                 GPIO_PIN_4
#define AUDIO_SAI_SD_GPIO_PORT           GPIOE
#define AUDIO_SAI_FS_PIN                 GPIO_PIN_5
#define AUDIO_SAI_FS_GPIO_PORT           GPIOE
#define AUDIO_SAI_SCK_PIN                GPIO_PIN_6
#define AUDIO_SAI_SCK_GPIO_PORT          GPIOE

/**
 * @brief I2S peripheral configuration
 */
#define AUDIO_I2S_INSTANCE               SPI3
#define AUDIO_I2S_CLK_ENABLE()           __HAL_RCC_SPI3_CLK_ENABLE()
#define AUDIO_I2S_CLK_DISABLE()          __HAL_RCC_SPI3_CLK_DISABLE()
#define AUDIO_I2S_WS_PIN                 GPIO_PIN_0
#define AUDIO_I2S_WS_GPIO_PORT           GPIOC
#define AUDIO_I2S_CK_PIN                 GPIO_PIN_10
#define AUDIO_I2S_CK_GPIO_PORT           GPIOC
#define AUDIO_I2S_SD_PIN                 GPIO_PIN_12
#define AUDIO_I2S_SD_GPIO_PORT           GPIOC

/**
 * @brief DMA configuration
 */
#define AUDIO_DMA_INSTANCE               DMA2_Stream1
#define AUDIO_DMA_CHANNEL                DMA_CHANNEL_0
#define AUDIO_DMA_CLK_ENABLE()           __HAL_RCC_DMA2_CLK_ENABLE()
#define AUDIO_DMA_IRQn                   DMA2_Stream1_IRQn
#define AUDIO_DMA_IRQHandler             DMA2_Stream1_IRQHandler

/**
 * @brief Audio PLL configuration
 */
#define AUDIO_PLL_M                      8U
#define AUDIO_PLL_N                      344U
#define AUDIO_PLL_P                      7U
#define AUDIO_PLL_Q                      7U

/**
 * @brief Codec control interface (Cirrus Logic CS43L22)
 * @note  Control is I2C, audio data is I2S/SAI. The address is the 8-bit write
 *        address; the CS43L22 7-bit address is 0x4A with AD0 tied low.
 */
#define AUDIO_CODEC_I2C_ADDRESS          0x94U
#define AUDIO_CODEC_I2C_TIMEOUT          I2C_TIMEOUT_DEFAULT
#define AUDIO_CODEC_RESET_GPIO_PORT      GPIOD
#define AUDIO_CODEC_RESET_PIN            GPIO_PIN_4

/**
 * @brief Codec constants
 */
#define AUDIO_VOLUME_SCALE               100U   /* Volume percentage scale */
#define AUDIO_CODEC_CHIP_ID              0xE0U  /* CS43L22 part number field */
#define AUDIO_CODEC_CHIP_ID_MASK         0xF8U  /* Upper 5 bits hold the part number */

/**
 * @brief SAI frame configuration
 */
#define AUDIO_SAI_FRAME_LENGTH           64U    /* SAI frame length */
#define AUDIO_SAI_ACTIVE_FRAME_LENGTH    32U    /* SAI active frame length */
#define AUDIO_SAI_RESET_DELAY            10U    /* Codec reset delay in ms */

/**
 * @brief Codec register values
 */
#define AUDIO_CODEC_POWER_DOWN           0x01U  /* Power control 1: powered down */
#define AUDIO_CODEC_POWER_UP             0x9EU  /* Power control 1: powered up */
#define AUDIO_CODEC_OUTPUT_HEADPHONE     0xAFU  /* Power control 2: headphone on, speaker off */
#define AUDIO_CODEC_CLOCK_AUTODETECT     0x81U  /* Clocking control: auto-detect MCLK ratio */
#define AUDIO_CODEC_INTERFACE_I2S        0x04U  /* Interface control 1: I2S slave, 16-bit */
#define AUDIO_CODEC_ANALOG_ZC_SR         0x00U  /* No zero-cross / soft-ramp on the analog path */
#define AUDIO_CODEC_HEADPHONE_MUTE       0xC0U  /* Playback control 2: mute both HP channels */
#define AUDIO_CODEC_HEADPHONE_UNMUTE     0x00U

/** @} */

/* Private types -------------------------------------------------------------*/

/** @defgroup AUDIO_Private_Types Private Types
 * @{
 */

/**
 * @brief Audio driver state
 */
typedef enum {
    AUDIO_STATE_RESET = 0,
    AUDIO_STATE_READY,
    AUDIO_STATE_PLAYING,
    AUDIO_STATE_PAUSED,
    AUDIO_STATE_ERROR
} AUDIO_StateTypeDef;

/**
 * @brief Audio codec registers (CS43L22)
 */
typedef enum {
    CODEC_REG_ID = 0x01,
    CODEC_REG_POWER_CTL1 = 0x02,
    CODEC_REG_POWER_CTL2 = 0x04,
    CODEC_REG_CLOCKING_CTL = 0x05,
    CODEC_REG_INTERFACE_CTL1 = 0x06,
    CODEC_REG_ANALOG_ZC_SR = 0x0A,
    CODEC_REG_PLAYBACK_CTL2 = 0x0F,
    CODEC_REG_MASTER_VOL_A = 0x20,
    CODEC_REG_MASTER_VOL_B = 0x21
} AUDIO_CodecRegTypeDef;

/** @} */

/* Private variables ---------------------------------------------------------*/

/** @defgroup AUDIO_Private_Variables Private Variables
 * @{
 */

static AUDIO_StateTypeDef AudioState = AUDIO_STATE_RESET;
static AUDIO_ConfigTypeDef AudioConfig;
static AUDIO_BufferTypeDef OutputBuffer;
static AUDIO_StatsTypeDef AudioStats;

static SAI_HandleTypeDef SaiHandle;
static I2S_HandleTypeDef I2sHandle;
static DMA_HandleTypeDef DmaHandle;

static uint8_t OutputBufferMemory[AUDIO_BUFFER_SIZE_DEFAULT];

static uint8_t CurrentVolume = AUDIO_VOLUME_DEFAULT;
static bool IsMuted = false;

/** @} */

/* Private function prototypes -----------------------------------------------*/

/** @defgroup AUDIO_Private_Functions Private Functions
 * @{
 */

static AUDIO_StatusTypeDef AUDIO_SAI_Init(void);
static AUDIO_StatusTypeDef AUDIO_I2S_Init(void);
static AUDIO_StatusTypeDef AUDIO_DMA_Init(void);
static AUDIO_StatusTypeDef AUDIO_Buffer_Init(void);
static AUDIO_StatusTypeDef AUDIO_Codec_WriteRegister(uint8_t reg, uint8_t value);
static AUDIO_StatusTypeDef AUDIO_Codec_ReadRegister(uint8_t reg, uint8_t *value);
static uint32_t AUDIO_SampleRateHz(AUDIO_FreqTypeDef rate);

/* DMA and interrupt handlers */
static void AUDIO_DMA_Complete_Callback(void);
static void AUDIO_DMA_Error_Callback(void);

/** @} */

/* Exported functions ---------------------------------------------------------*/

/** @defgroup AUDIO_Exported_Functions Exported Functions
 * @{
 */

/**
 * @brief   Initialize the audio subsystem
 * @details Configures audio peripherals and initializes the audio system
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Init(void)
{
    AUDIO_ConfigTypeDef defaultConfig = {
        .Interface = AUDIO_INTERFACE_SAI,
        .SampleRate = AUDIO_FREQ_44K,
        .BitDepth = AUDIO_FORMAT_16BIT,
        .Channels = AUDIO_CHANNEL_STEREO,
        .BufferSize = AUDIO_BUFFER_SIZE_DEFAULT,
        .EnableDMA = true
    };

    return AUDIO_Init_Custom(&defaultConfig);
}

/**
 * @brief   Initialize audio with custom configuration
 * @details Allows custom configuration of audio parameters
 * @param   config Pointer to audio configuration structure
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Init_Custom(const AUDIO_ConfigTypeDef* config)
{
    AUDIO_StatusTypeDef status = AUDIO_OK;

    if (config == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    /* Copy configuration */
    memcpy(&AudioConfig, config, sizeof(AUDIO_ConfigTypeDef));

    /* Reset statistics */
    memset(&AudioStats, 0, sizeof(AUDIO_StatsTypeDef));

    /* Initialize buffers */
    status = AUDIO_Buffer_Init();
    if (status != AUDIO_OK) {
        return status;
    }

    /* Initialize DMA if enabled */
    if (AudioConfig.EnableDMA) {
        status = AUDIO_DMA_Init();
        if (status != AUDIO_OK) {
            return status;
        }
    }

    /* Initialize audio interface */
    switch (AudioConfig.Interface) {
        case AUDIO_INTERFACE_SAI:
            status = AUDIO_SAI_Init();
            break;
        case AUDIO_INTERFACE_I2S:
            status = AUDIO_I2S_Init();
            break;
        default:
            status = AUDIO_INVALID_PARAM;
            break;
    }

    if (status != AUDIO_OK) {
        return status;
    }

    /* Initialize codec */
    status = AUDIO_CodecInit();
    if (status != AUDIO_OK) {
        return status;
    }

    /* Set initial state */
    AudioState = AUDIO_STATE_READY;

    return AUDIO_OK;
}

/**
 * @brief   Deinitialize the audio subsystem
 * @details Stops audio processing and releases resources
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_DeInit(void)
{
    AUDIO_StatusTypeDef status = AUDIO_OK;

    /* Stop any ongoing operations */
    AUDIO_Stop();

    /* Deinitialize audio interface */
    switch (AudioConfig.Interface) {
        case AUDIO_INTERFACE_SAI:
            HAL_SAI_DeInit(&SaiHandle);
            AUDIO_SAI_CLK_DISABLE();
            break;
        case AUDIO_INTERFACE_I2S:
            HAL_I2S_DeInit(&I2sHandle);
            AUDIO_I2S_CLK_DISABLE();
            break;
        default:
            break;
    }

    /* Deinitialize DMA */
    if (AudioConfig.EnableDMA) {
        HAL_DMA_DeInit(&DmaHandle);
    }

    /* Detach the static buffers */
    OutputBuffer.Buffer = NULL;

    /* Reset state */
    AudioState = AUDIO_STATE_RESET;

    return status;
}

/**
 * @brief   Start audio playback
 * @details Begins audio output from the configured buffer
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Play(void)
{
    if (AudioState != AUDIO_STATE_READY && AudioState != AUDIO_STATE_PAUSED) {
        return AUDIO_NOT_READY;
    }

    /* Start audio interface */
    switch (AudioConfig.Interface) {
        case AUDIO_INTERFACE_SAI:
            if (HAL_SAI_Transmit_DMA(&SaiHandle, OutputBuffer.Buffer,
                                   OutputBuffer.Size / 2) != HAL_OK) {
                return AUDIO_ERROR;
            }
            break;
        case AUDIO_INTERFACE_I2S:
            if (HAL_I2S_Transmit_DMA(&I2sHandle, (uint16_t*)OutputBuffer.Buffer,
                                   OutputBuffer.Size / 2) != HAL_OK) {
                return AUDIO_ERROR;
            }
            break;
        default:
            return AUDIO_ERROR;
    }

    AudioState = AUDIO_STATE_PLAYING;
    return AUDIO_OK;
}

/**
 * @brief   Stop audio playback
 * @details Stops audio output and flushes buffers
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Stop(void)
{
    if (AudioState != AUDIO_STATE_PLAYING && AudioState != AUDIO_STATE_PAUSED) {
        return AUDIO_NOT_READY;
    }

    /* Stop audio interface */
    switch (AudioConfig.Interface) {
        case AUDIO_INTERFACE_SAI:
            HAL_SAI_DMAStop(&SaiHandle);
            break;
        case AUDIO_INTERFACE_I2S:
            HAL_I2S_DMAStop(&I2sHandle);
            break;
        default:
            break;
    }

    /* Reset buffer indices */
    OutputBuffer.ReadIndex = 0;
    OutputBuffer.WriteIndex = 0;
    OutputBuffer.IsEmpty = true;
    OutputBuffer.IsFull = false;

    AudioState = AUDIO_STATE_READY;
    return AUDIO_OK;
}

/**
 * @brief   Pause audio playback
 * @details Pauses audio output without flushing buffers
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Pause(void)
{
    if (AudioState != AUDIO_STATE_PLAYING) {
        return AUDIO_NOT_READY;
    }

    /* Pause audio interface */
    switch (AudioConfig.Interface) {
        case AUDIO_INTERFACE_SAI:
            HAL_SAI_DMAPause(&SaiHandle);
            break;
        case AUDIO_INTERFACE_I2S:
            HAL_I2S_DMAPause(&I2sHandle);
            break;
        default:
            break;
    }

    AudioState = AUDIO_STATE_PAUSED;
    return AUDIO_OK;
}

/**
 * @brief   Resume audio playback
 * @details Resumes audio output from paused state
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_Resume(void)
{
    if (AudioState != AUDIO_STATE_PAUSED) {
        return AUDIO_NOT_READY;
    }

    /* Resume audio interface */
    switch (AudioConfig.Interface) {
        case AUDIO_INTERFACE_SAI:
            HAL_SAI_DMAResume(&SaiHandle);
            break;
        case AUDIO_INTERFACE_I2S:
            HAL_I2S_DMAResume(&I2sHandle);
            break;
        default:
            break;
    }

    AudioState = AUDIO_STATE_PLAYING;
    return AUDIO_OK;
}

/**
 * @brief   Write audio data to output buffer
 * @details Adds audio samples to the playback buffer
 * @param   data Pointer to audio data buffer
 * @param   size Size of data in bytes
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_WriteBuffer(uint8_t* data, uint32_t size)
{
    uint32_t bytesWritten = 0;
    uint32_t spaceAvailable = 0;

    if (data == NULL || size == 0) {
        return AUDIO_INVALID_PARAM;
    }

    /* Equal indices mean either full or empty, so the flag decides. */
    if (OutputBuffer.IsFull) {
        spaceAvailable = 0;
    } else if (OutputBuffer.WriteIndex >= OutputBuffer.ReadIndex) {
        spaceAvailable = OutputBuffer.Size - (OutputBuffer.WriteIndex - OutputBuffer.ReadIndex);
    } else {
        spaceAvailable = OutputBuffer.ReadIndex - OutputBuffer.WriteIndex;
    }

    if (size > spaceAvailable) {
        AudioStats.BufferOverflows++;
        return AUDIO_OVERFLOW;
    }

    /* Copy data to buffer */
    while (bytesWritten < size) {
        OutputBuffer.Buffer[OutputBuffer.WriteIndex] = data[bytesWritten];
        OutputBuffer.WriteIndex = (OutputBuffer.WriteIndex + 1) % OutputBuffer.Size;
        bytesWritten++;
    }

    OutputBuffer.IsEmpty = false;
    if (OutputBuffer.WriteIndex == OutputBuffer.ReadIndex) {
        OutputBuffer.IsFull = true;
    }

    return AUDIO_OK;
}

/**
 * @brief   Set audio volume
 * @details Adjusts the audio output volume level
 * @param   volume Volume level (0-100)
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_SetVolume(uint8_t volume)
{
    if (volume > AUDIO_VOLUME_MAX) {
        volume = AUDIO_VOLUME_MAX;
    }

    /* Master volume is a 256-code ramp that wraps: 0x19 is the quietest, it
       climbs to 0xFF (-0.5 dB), then 0x00 (0 dB) up to 0x18 (+12 dB). */
    uint8_t code = (uint8_t)(((uint32_t)volume * 255U) / AUDIO_VOLUME_SCALE);
    uint8_t regValue = (code > 0xE6U) ? (uint8_t)(code - 0xE7U) : (uint8_t)(code + 0x19U);

    AUDIO_StatusTypeDef status = AUDIO_Codec_WriteRegister(CODEC_REG_MASTER_VOL_A, regValue);
    if (status != AUDIO_OK) {
        return status;
    }

    status = AUDIO_Codec_WriteRegister(CODEC_REG_MASTER_VOL_B, regValue);
    if (status != AUDIO_OK) {
        return status;
    }

    CurrentVolume = volume;

    return AUDIO_OK;
}

/**
 * @brief   Get current audio volume
 * @details Returns the current volume level
 * @param   volume Pointer to store volume level
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetVolume(uint8_t* volume)
{
    if (volume == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    *volume = CurrentVolume;
    return AUDIO_OK;
}

/**
 * @brief   Set audio mute state
 * @details Mutes or unmutes audio output
 * @param   mute True to mute, false to unmute
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_SetMute(bool mute)
{
    uint8_t regValue = mute ? AUDIO_CODEC_HEADPHONE_MUTE : AUDIO_CODEC_HEADPHONE_UNMUTE;

    AUDIO_StatusTypeDef status = AUDIO_Codec_WriteRegister(CODEC_REG_PLAYBACK_CTL2, regValue);
    if (status != AUDIO_OK) {
        return status;
    }

    IsMuted = mute;

    return AUDIO_OK;
}

/**
 * @brief   Get audio mute state
 * @details Returns the current mute state
 * @param   mute Pointer to store mute state
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetMute(bool* mute)
{
    if (mute == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    *mute = IsMuted;
    return AUDIO_OK;
}

/**
 * @brief   Get audio system status
 * @details Returns current audio system state
 * @param   None
 * @retval  AUDIO_StatusTypeDef Current system status
 */
AUDIO_StatusTypeDef AUDIO_GetStatus(void)
{
    return (AudioState == AUDIO_STATE_ERROR) ? AUDIO_ERROR : AUDIO_OK;
}

/**
 * @brief   Get audio statistics
 * @details Returns performance and error statistics
 * @param   stats Pointer to statistics structure
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_GetStatistics(AUDIO_StatsTypeDef* stats)
{
    if (stats == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    memcpy(stats, &AudioStats, sizeof(AUDIO_StatsTypeDef));
    return AUDIO_OK;
}

/**
 * @brief   Get audio status string
 * @details Converts status code to human-readable string
 * @param   status Audio status code
 * @retval  const char* Status description string
 */
const char* AUDIO_GetStatusString(AUDIO_StatusTypeDef status)
{
    switch (status) {
        case AUDIO_OK:              return "OK";
        case AUDIO_ERROR:           return "Error";
        case AUDIO_BUSY:            return "Busy";
        case AUDIO_TIMEOUT:         return "Timeout";
        case AUDIO_INVALID_PARAM:   return "Invalid Parameter";
        case AUDIO_NOT_READY:       return "Not Ready";
        case AUDIO_OVERFLOW:        return "Buffer Overflow";
        case AUDIO_UNDERFLOW:       return "Buffer Underflow";
        default:                    return "Unknown";
    }
}

/* Private functions ---------------------------------------------------------*/

/** @defgroup AUDIO_Private_Functions Private Functions Implementation
 * @{
 */

/**
 * @brief   Initialize SAI peripheral
 * @details Configures SAI for audio transmission/reception
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
static AUDIO_StatusTypeDef AUDIO_SAI_Init(void)
{
    AUDIO_SAI_CLK_ENABLE();
    /* GPIO driver enables the GPIOE port clock */

    /* Configure SAI pins */
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.Pin = AUDIO_SAI_MCK_PIN | AUDIO_SAI_SD_PIN |
                   AUDIO_SAI_FS_PIN | AUDIO_SAI_SCK_PIN;
    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpioInit.Alternate = GPIO_AF6_SAI1;
    GPIO_Driver_Pin_Init(AUDIO_SAI_MCK_GPIO_PORT, &gpioInit);

    /* Configure SAI */
    SaiHandle.Instance = AUDIO_SAI_INSTANCE;
    SaiHandle.Init.AudioMode = SAI_MODEMASTER_TX;
    SaiHandle.Init.Synchro = SAI_ASYNCHRONOUS;
    SaiHandle.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
    SaiHandle.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    SaiHandle.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
    SaiHandle.Init.AudioFrequency = AUDIO_SampleRateHz(AudioConfig.SampleRate);
    SaiHandle.Init.Protocol = SAI_FREE_PROTOCOL;
    SaiHandle.Init.DataSize = SAI_DATASIZE_16;
    SaiHandle.Init.FirstBit = SAI_FIRSTBIT_MSB;
    SaiHandle.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;

    SaiHandle.FrameInit.FrameLength = AUDIO_SAI_FRAME_LENGTH;
    SaiHandle.FrameInit.ActiveFrameLength = AUDIO_SAI_ACTIVE_FRAME_LENGTH;
    SaiHandle.FrameInit.FSDefinition = SAI_FS_STARTFRAME;
    SaiHandle.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
    SaiHandle.FrameInit.FSOffset = SAI_FS_FIRSTBIT;

    SaiHandle.SlotInit.FirstBitOffset = 0;
    SaiHandle.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
    SaiHandle.SlotInit.SlotNumber = 2;
    SaiHandle.SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

    if (HAL_SAI_Init(&SaiHandle) != HAL_OK) {
        return AUDIO_ERROR;
    }

    return AUDIO_OK;
}

/**
 * @brief   Initialize I2S peripheral
 * @details Configures I2S for audio transmission/reception
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
static AUDIO_StatusTypeDef AUDIO_I2S_Init(void)
{
    AUDIO_I2S_CLK_ENABLE();
    /* GPIO driver enables the GPIOC port clock */

    /* Configure I2S pins */
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.Pin = AUDIO_I2S_WS_PIN | AUDIO_I2S_CK_PIN | AUDIO_I2S_SD_PIN;
    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpioInit.Alternate = GPIO_AF6_SPI3;
    GPIO_Driver_Pin_Init(AUDIO_I2S_WS_GPIO_PORT, &gpioInit);

    /* Configure I2S */
    I2sHandle.Instance = AUDIO_I2S_INSTANCE;
    I2sHandle.Init.Mode = I2S_MODE_MASTER_TX;
    I2sHandle.Init.Standard = I2S_STANDARD_PHILIPS;
    I2sHandle.Init.DataFormat = I2S_DATAFORMAT_16B;
    I2sHandle.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    I2sHandle.Init.AudioFreq = AUDIO_SampleRateHz(AudioConfig.SampleRate);
    I2sHandle.Init.CPOL = I2S_CPOL_LOW;
    I2sHandle.Init.ClockSource = I2S_CLOCK_PLL;
    I2sHandle.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

    if (HAL_I2S_Init(&I2sHandle) != HAL_OK) {
        return AUDIO_ERROR;
    }

    return AUDIO_OK;
}

/**
 * @brief   Initialize DMA for audio
 * @details Configures DMA for audio data transfer
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
static AUDIO_StatusTypeDef AUDIO_DMA_Init(void)
{
    AUDIO_DMA_CLK_ENABLE();

    /* Configure DMA */
    DmaHandle.Instance = AUDIO_DMA_INSTANCE;
    DmaHandle.Init.Channel = AUDIO_DMA_CHANNEL;
    DmaHandle.Init.Direction = DMA_MEMORY_TO_PERIPH;
    DmaHandle.Init.PeriphInc = DMA_PINC_DISABLE;
    DmaHandle.Init.MemInc = DMA_MINC_ENABLE;
    DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    DmaHandle.Init.Mode = DMA_CIRCULAR;
    DmaHandle.Init.Priority = DMA_PRIORITY_HIGH;
    DmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    DmaHandle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    DmaHandle.Init.MemBurst = DMA_MBURST_SINGLE;
    DmaHandle.Init.PeriphBurst = DMA_PBURST_SINGLE;

    if (HAL_DMA_Init(&DmaHandle) != HAL_OK) {
        return AUDIO_ERROR;
    }

    /* Link DMA to SAI/I2S */
    if (AudioConfig.Interface == AUDIO_INTERFACE_SAI) {
        __HAL_LINKDMA(&SaiHandle, hdmatx, DmaHandle);
    } else {
        __HAL_LINKDMA(&I2sHandle, hdmatx, DmaHandle);
    }

    /* Configure DMA interrupts */
    HAL_NVIC_SetPriority(AUDIO_DMA_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(AUDIO_DMA_IRQn);

    return AUDIO_OK;
}

/**
 * @brief   Initialize audio buffers
 * @details Binds the static buffer memory to the input/output ring buffers
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
static AUDIO_StatusTypeDef AUDIO_Buffer_Init(void)
{
    if (AudioConfig.BufferSize > sizeof(OutputBufferMemory)) {
        return AUDIO_ERROR;
    }

    /* Initialize output buffer structure */
    OutputBuffer.Buffer = OutputBufferMemory;
    OutputBuffer.Size = AudioConfig.BufferSize;
    OutputBuffer.ReadIndex = 0;
    OutputBuffer.WriteIndex = 0;
    OutputBuffer.IsEmpty = true;
    OutputBuffer.IsFull = false;

    return AUDIO_OK;
}

/**
 * @brief   Initialize audio codec
 * @details Brings the CS43L22 out of reset and configures it for I2S playback
 * @param   None
 * @retval  AUDIO_StatusTypeDef Operation status
 */
AUDIO_StatusTypeDef AUDIO_CodecInit(void)
{
    static const struct {
        uint8_t reg;
        uint8_t value;
    } codecInitSequence[] = {
        {CODEC_REG_POWER_CTL1,     AUDIO_CODEC_POWER_DOWN},
        {CODEC_REG_POWER_CTL2,     AUDIO_CODEC_OUTPUT_HEADPHONE},
        {CODEC_REG_CLOCKING_CTL,   AUDIO_CODEC_CLOCK_AUTODETECT},
        {CODEC_REG_INTERFACE_CTL1, AUDIO_CODEC_INTERFACE_I2S},
        {CODEC_REG_ANALOG_ZC_SR,   AUDIO_CODEC_ANALOG_ZC_SR},
    };

    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = AUDIO_CODEC_RESET_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_Driver_Pin_Init(AUDIO_CODEC_RESET_GPIO_PORT, &GPIO_InitStruct);

    /* RESET is active low and must be released before the codec answers on I2C. */
    HAL_GPIO_WritePin(AUDIO_CODEC_RESET_GPIO_PORT, AUDIO_CODEC_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(AUDIO_SAI_RESET_DELAY);
    HAL_GPIO_WritePin(AUDIO_CODEC_RESET_GPIO_PORT, AUDIO_CODEC_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(AUDIO_SAI_RESET_DELAY);

    I2C_Init();

    uint8_t chipId = 0;
    AUDIO_StatusTypeDef status = AUDIO_Codec_ReadRegister(CODEC_REG_ID, &chipId);
    if (status != AUDIO_OK) {
        return status;
    }
    if ((chipId & AUDIO_CODEC_CHIP_ID_MASK) != AUDIO_CODEC_CHIP_ID) {
        return AUDIO_ERROR;
    }

    for (uint32_t i = 0; i < (sizeof(codecInitSequence) / sizeof(codecInitSequence[0])); i++) {
        status = AUDIO_Codec_WriteRegister(codecInitSequence[i].reg, codecInitSequence[i].value);
        if (status != AUDIO_OK) {
            return status;
        }
    }

    status = AUDIO_SetVolume(CurrentVolume);
    if (status != AUDIO_OK) {
        return status;
    }

    return AUDIO_Codec_WriteRegister(CODEC_REG_POWER_CTL1, AUDIO_CODEC_POWER_UP);
}

/**
 * @brief   Write to codec register
 * @details Writes a value to a codec register via I2C
 * @param   reg Register address
 * @param   value Value to write
 * @retval  AUDIO_StatusTypeDef Operation status
 */
static AUDIO_StatusTypeDef AUDIO_Codec_WriteRegister(uint8_t reg, uint8_t value)
{
    if (I2C_Mem_Write(AUDIO_CODEC_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT,
                      &value, 1, AUDIO_CODEC_I2C_TIMEOUT) != I2C_OK) {
        return AUDIO_ERROR;
    }

    return AUDIO_OK;
}

/**
 * @brief   Read a codec register
 * @param   reg Register address
 * @param   value Pointer to store the register contents
 * @retval  AUDIO_StatusTypeDef Operation status
 */
static AUDIO_StatusTypeDef AUDIO_Codec_ReadRegister(uint8_t reg, uint8_t *value)
{
    if (value == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    if (I2C_Mem_Read(AUDIO_CODEC_I2C_ADDRESS, reg, I2C_MEMADD_SIZE_8BIT,
                     value, 1, AUDIO_CODEC_I2C_TIMEOUT) != I2C_OK) {
        return AUDIO_ERROR;
    }

    return AUDIO_OK;
}

/**
 * @brief   Translate the configured sample rate to hertz
 * @param   rate Configured sample rate
 * @retval  uint32_t Sample rate in Hz
 */
static uint32_t AUDIO_SampleRateHz(AUDIO_FreqTypeDef rate)
{
    switch (rate) {
        case AUDIO_FREQ_8K:  return 8000U;
        case AUDIO_FREQ_11K: return 11025U;
        case AUDIO_FREQ_16K: return 16000U;
        case AUDIO_FREQ_22K: return 22050U;
        case AUDIO_FREQ_32K: return 32000U;
        case AUDIO_FREQ_48K: return 48000U;
        case AUDIO_FREQ_96K: return 96000U;
        case AUDIO_FREQ_44K:
        default:             return 44100U;
    }
}

/**
 * @brief   DMA transfer complete callback
 * @details Advances the output ring buffer by one half-buffer
 * @param   None
 * @retval  None
 */
static void AUDIO_DMA_Complete_Callback(void)
{
    /* Update buffer read index */
    OutputBuffer.ReadIndex = (OutputBuffer.ReadIndex + OutputBuffer.Size / 2) % OutputBuffer.Size;

    if (OutputBuffer.ReadIndex == OutputBuffer.WriteIndex) {
        OutputBuffer.IsEmpty = true;
    }
    OutputBuffer.IsFull = false;

    uint32_t bytesPerSample = (AudioConfig.BitDepth == AUDIO_FORMAT_16BIT) ? 2U : 4U;
    if (AudioConfig.Channels == AUDIO_CHANNEL_STEREO) {
        bytesPerSample *= 2U;
    }
    AudioStats.SampleCount += (OutputBuffer.Size / 2U) / bytesPerSample;
}

/**
 * @brief   DMA error callback
 * @details Called when DMA transfer encounters an error
 * @param   None
 * @retval  None
 */
static void AUDIO_DMA_Error_Callback(void)
{
    AudioState = AUDIO_STATE_ERROR;
    AudioStats.SyncErrors++;
}

/* HAL transfer callbacks — only act on handles owned by this driver */

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == &SaiHandle) {
        AUDIO_DMA_Complete_Callback();
    }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
    if (hsai == &SaiHandle) {
        AUDIO_DMA_Error_Callback();
    }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s == &I2sHandle) {
        AUDIO_DMA_Complete_Callback();
    }
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
    if (hi2s == &I2sHandle) {
        AUDIO_DMA_Error_Callback();
    }
}

/** @} */

/** @} */
