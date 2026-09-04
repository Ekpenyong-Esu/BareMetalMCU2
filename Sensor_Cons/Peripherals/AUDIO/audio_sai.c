/**
 ******************************************************************************
 * @file    audio_sai.c
 * @brief   SAI transport backend for the audio subsystem
 * @details Programs whichever SAI block and pins the config names. Nothing
 *          here knows which board it runs on.
 ******************************************************************************
 */

#include "audio_sai.h"
#include "gpio.h"

/* Private defines -----------------------------------------------------------*/

#define AUDIO_SAI_FRAME_LENGTH 64U        /* SAI frame length */
#define AUDIO_SAI_ACTIVE_FRAME_LENGTH 32U /* SAI active frame length */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Alternate function that routes a SAI block to its pins
 * @retval  bool false when the block is not one this part has
 */
static bool Audio_SaiResolveAlternate(const SAI_Block_TypeDef *block, uint8_t *alternate) {
    if (block == SAI1_Block_A || block == SAI1_Block_B) {
        *alternate = GPIO_AF6_SAI1;
        return true;
    }
    return false;
}

/**
 * @brief   Put one wired line into alternate-function mode
 */
static void Audio_SaiInitPin(const AUDIO_Pin_t *line, uint8_t alternate) {
    if (!Audio_PinIsWired(line)) {
        return;
    }

    /* The GPIO driver enables the port clock. */
    GPIO_InitTypeDef gpioInit = {0};
    gpioInit.Pin = line->pin;
    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    gpioInit.Alternate = alternate;
    (void)GPIO_Driver_Pin_Init(line->port, &gpioInit);
}

static void Audio_SaiDeInitPin(const AUDIO_Pin_t *line) {
    if (Audio_PinIsWired(line)) {
        (void)GPIO_Driver_Pin_DeInit(line->port, line->pin);
    }
}

static AUDIO_StatusTypeDef Audio_SaiInit(AUDIO_Handle_t *dev) {
    const AUDIO_ConfigTypeDef *cfg = &dev->config;
    uint8_t alternate = cfg->alternate;

    if (cfg->saiBlock == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    if (alternate == 0U && !Audio_SaiResolveAlternate(cfg->saiBlock, &alternate)) {
        return AUDIO_INVALID_PARAM;
    }

    /* Both blocks share the SAI1 clock; this part has no SAI2. */
    __HAL_RCC_SAI1_CLK_ENABLE();
    Audio_SaiInitPin(&cfg->wsPin, alternate);
    Audio_SaiInitPin(&cfg->ckPin, alternate);
    Audio_SaiInitPin(&cfg->sdPin, alternate);
    Audio_SaiInitPin(&cfg->mclkPin, alternate);

    dev->sai.Instance = cfg->saiBlock;
    dev->sai.Init.AudioMode = SAI_MODEMASTER_TX;
    dev->sai.Init.Synchro = SAI_ASYNCHRONOUS;
    dev->sai.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
    dev->sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLE;
    dev->sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
    dev->sai.Init.AudioFrequency = Audio_SampleRateHz(cfg->SampleRate);
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

static void Audio_SaiDeInit(AUDIO_Handle_t *dev) {
    const AUDIO_ConfigTypeDef *cfg = &dev->config;

    (void)HAL_SAI_DeInit(&dev->sai);
    __HAL_RCC_SAI1_CLK_DISABLE();

    Audio_SaiDeInitPin(&cfg->wsPin);
    Audio_SaiDeInitPin(&cfg->ckPin);
    Audio_SaiDeInitPin(&cfg->sdPin);
    Audio_SaiDeInitPin(&cfg->mclkPin);
}

static void Audio_SaiLinkDma(AUDIO_Handle_t *dev) {
    __HAL_LINKDMA(&dev->sai, hdmatx, dev->dma);
}

static AUDIO_StatusTypeDef Audio_SaiStart(AUDIO_Handle_t *dev) {
    /* The DMA moves half-words, so the transfer count is half the byte count. */
    if (HAL_SAI_Transmit_DMA(&dev->sai, dev->output.Buffer, (uint16_t)(dev->output.Size / 2U)) !=
        HAL_OK) {
        return AUDIO_ERROR;
    }
    return AUDIO_OK;
}

static void Audio_SaiStop(AUDIO_Handle_t *dev) {
    (void)HAL_SAI_DMAStop(&dev->sai);
}

static void Audio_SaiPause(AUDIO_Handle_t *dev) {
    (void)HAL_SAI_DMAPause(&dev->sai);
}

static void Audio_SaiResume(AUDIO_Handle_t *dev) {
    (void)HAL_SAI_DMAResume(&dev->sai);
}

/* Public data ---------------------------------------------------------------*/

const AudioIfOps_t AudioSaiOps = {
    .name = "SAI",
    .init = Audio_SaiInit,
    .deinit = Audio_SaiDeInit,
    .linkDma = Audio_SaiLinkDma,
    .start = Audio_SaiStart,
    .stop = Audio_SaiStop,
    .pause = Audio_SaiPause,
    .resume = Audio_SaiResume,
};
