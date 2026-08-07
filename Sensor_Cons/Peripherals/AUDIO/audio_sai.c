/**
  ******************************************************************************
  * @file    audio_sai.c
  * @brief   SAI transport backend for the audio subsystem
  * @details Owns the SAI1 Block A pin map, peripheral configuration and DMA
  *          stream selection. Nothing outside this file knows these details.
  ******************************************************************************
  */

#include "audio_sai.h"
#include "gpio.h"

/* Private defines -----------------------------------------------------------*/

#define AUDIO_SAI_INSTANCE               SAI1_Block_A
#define AUDIO_SAI_CLK_ENABLE()           __HAL_RCC_SAI1_CLK_ENABLE()
#define AUDIO_SAI_CLK_DISABLE()          __HAL_RCC_SAI1_CLK_DISABLE()

#define AUDIO_SAI_GPIO_PORT              GPIOE
#define AUDIO_SAI_MCK_PIN                GPIO_PIN_2
#define AUDIO_SAI_SD_PIN                 GPIO_PIN_4
#define AUDIO_SAI_FS_PIN                 GPIO_PIN_5
#define AUDIO_SAI_SCK_PIN                GPIO_PIN_6

/**
 * @brief SAI1_A transmit DMA
 * @note  RM0090 offers DMA2 Stream 1 and Stream 3 on channel 0 for SAI1_A.
 *        Stream 1 is already claimed by the MIC driver, so Stream 3 is used.
 */
#define AUDIO_SAI_DMA_STREAM             DMA2_Stream3
#define AUDIO_SAI_DMA_CHANNEL            DMA_CHANNEL_0
#define AUDIO_SAI_DMA_IRQn               DMA2_Stream3_IRQn

#define AUDIO_SAI_FRAME_LENGTH           64U    /* SAI frame length */
#define AUDIO_SAI_ACTIVE_FRAME_LENGTH    32U    /* SAI active frame length */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Route the SAI1 Block A signals to their pins
 * @retval  None
 */
static void Audio_SaiInitPins(void)
{
    /* The GPIO driver enables the port clock. */
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.Pin = AUDIO_SAI_MCK_PIN | AUDIO_SAI_SD_PIN |
                   AUDIO_SAI_FS_PIN | AUDIO_SAI_SCK_PIN;
    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpioInit.Alternate = GPIO_AF6_SAI1;
    (void)GPIO_Driver_Pin_Init(AUDIO_SAI_GPIO_PORT, &gpioInit);
}

static AUDIO_StatusTypeDef Audio_SaiInit(AudioDevice_t* dev)
{
    AUDIO_SAI_CLK_ENABLE();
    Audio_SaiInitPins();

    dev->sai.Instance = AUDIO_SAI_INSTANCE;
    dev->sai.Init.AudioMode = SAI_MODEMASTER_TX;
    dev->sai.Init.Synchro = SAI_ASYNCHRONOUS;
    dev->sai.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
    dev->sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    dev->sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
    dev->sai.Init.AudioFrequency = Audio_SampleRateHz(dev->config.SampleRate);
    dev->sai.Init.Protocol = SAI_FREE_PROTOCOL;
    dev->sai.Init.DataSize = SAI_DATASIZE_16;
    dev->sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
    dev->sai.Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;

    dev->sai.FrameInit.FrameLength = AUDIO_SAI_FRAME_LENGTH;
    dev->sai.FrameInit.ActiveFrameLength = AUDIO_SAI_ACTIVE_FRAME_LENGTH;
    dev->sai.FrameInit.FSDefinition = SAI_FS_STARTFRAME;
    dev->sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
    dev->sai.FrameInit.FSOffset = SAI_FS_FIRSTBIT;

    dev->sai.SlotInit.FirstBitOffset = 0;
    dev->sai.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
    dev->sai.SlotInit.SlotNumber = 2;
    dev->sai.SlotInit.SlotActive = SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1;

    if (HAL_SAI_Init(&dev->sai) != HAL_OK) {
        return AUDIO_ERROR;
    }

    return AUDIO_OK;
}

static void Audio_SaiDeInit(AudioDevice_t* dev)
{
    (void)HAL_SAI_DeInit(&dev->sai);
    AUDIO_SAI_CLK_DISABLE();
}

static void Audio_SaiLinkDma(AudioDevice_t* dev)
{
    __HAL_LINKDMA(&dev->sai, hdmatx, dev->dma);
}

static AUDIO_StatusTypeDef Audio_SaiStart(AudioDevice_t* dev)
{
    /* The DMA moves half-words, so the transfer count is half the byte count. */
    if (HAL_SAI_Transmit_DMA(&dev->sai, dev->output.Buffer,
                             (uint16_t)(dev->output.Size / 2U)) != HAL_OK) {
        return AUDIO_ERROR;
    }
    return AUDIO_OK;
}

static void Audio_SaiStop(AudioDevice_t* dev)
{
    (void)HAL_SAI_DMAStop(&dev->sai);
}

static void Audio_SaiPause(AudioDevice_t* dev)
{
    (void)HAL_SAI_DMAPause(&dev->sai);
}

static void Audio_SaiResume(AudioDevice_t* dev)
{
    (void)HAL_SAI_DMAResume(&dev->sai);
}

/* Public data ---------------------------------------------------------------*/

const AudioIfOps_t AudioSaiOps = {
    .name       = "SAI",
    .dmaStream  = AUDIO_SAI_DMA_STREAM,
    .dmaChannel = AUDIO_SAI_DMA_CHANNEL,
    .dmaIrq     = AUDIO_SAI_DMA_IRQn,
    .init       = Audio_SaiInit,
    .deinit     = Audio_SaiDeInit,
    .linkDma    = Audio_SaiLinkDma,
    .start      = Audio_SaiStart,
    .stop       = Audio_SaiStop,
    .pause      = Audio_SaiPause,
    .resume     = Audio_SaiResume,
};
