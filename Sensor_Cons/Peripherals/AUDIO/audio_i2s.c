/**
 ******************************************************************************
 * @file    audio_i2s.c
 * @brief   I2S transport backend for the audio subsystem
 * @details Programs whichever SPI/I2S instance and pins the config names.
 *          Nothing here knows which board it runs on.
 ******************************************************************************
 */

#include "audio_i2s.h"
#include "gpio.h"

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Alternate function that routes an I2S instance to its pins
 * @retval  bool false when the instance cannot do I2S
 */
static bool Audio_I2sResolveAlternate(const SPI_TypeDef *instance, uint8_t *alternate) {
    if (instance == SPI2) {
        *alternate = GPIO_AF5_SPI2;
        return true;
    }
    if (instance == SPI3) {
        *alternate = GPIO_AF6_SPI3;
        return true;
    }
    /* SPI1/4/5/6 have no I2S mode on this part. */
    return false;
}

/**
 * @brief   Gate the clock of the instance on or off
 */
static void Audio_I2sClock(const SPI_TypeDef *instance, bool enable) {
    if (instance == SPI2) {
        if (enable) {
            __HAL_RCC_SPI2_CLK_ENABLE();
        }
        else {
            __HAL_RCC_SPI2_CLK_DISABLE();
        }
    }
    else {
        if (enable) {
            __HAL_RCC_SPI3_CLK_ENABLE();
        }
        else {
            __HAL_RCC_SPI3_CLK_DISABLE();
        }
    }
}

/**
 * @brief   Put one wired line into alternate-function mode
 */
static void Audio_I2sInitPin(const AUDIO_Pin_t *line, uint8_t alternate) {
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

static void Audio_I2sDeInitPin(const AUDIO_Pin_t *line) {
    if (Audio_PinIsWired(line)) {
        (void)GPIO_Driver_Pin_DeInit(line->port, line->pin);
    }
}

static AUDIO_StatusTypeDef Audio_I2sInit(AUDIO_Handle_t *dev) {
    const AUDIO_ConfigTypeDef *cfg = &dev->config;
    uint8_t alternate = cfg->alternate;

    if (cfg->i2sInstance == NULL) {
        return AUDIO_INVALID_PARAM;
    }
    if (alternate == 0U && !Audio_I2sResolveAlternate(cfg->i2sInstance, &alternate)) {
        return AUDIO_INVALID_PARAM;
    }

    Audio_I2sClock(cfg->i2sInstance, true);
    Audio_I2sInitPin(&cfg->wsPin, alternate);
    Audio_I2sInitPin(&cfg->ckPin, alternate);
    Audio_I2sInitPin(&cfg->sdPin, alternate);
    Audio_I2sInitPin(&cfg->mclkPin, alternate);

    dev->i2s.Instance = cfg->i2sInstance;
    dev->i2s.Init.Mode = I2S_MODE_MASTER_TX;
    dev->i2s.Init.Standard = I2S_STANDARD_PHILIPS;
    dev->i2s.Init.DataFormat = I2S_DATAFORMAT_16B;
    /* A codec without an MCLK input has nowhere for the clock to go. */
    dev->i2s.Init.MCLKOutput =
        Audio_PinIsWired(&cfg->mclkPin) ? I2S_MCLKOUTPUT_ENABLE : I2S_MCLKOUTPUT_DISABLE;
    dev->i2s.Init.AudioFreq = Audio_SampleRateHz(cfg->SampleRate);
    dev->i2s.Init.CPOL = I2S_CPOL_LOW;
    dev->i2s.Init.ClockSource = I2S_CLOCK_PLL;
    dev->i2s.Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

    if (HAL_I2S_Init(&dev->i2s) != HAL_OK) {
        return AUDIO_ERROR;
    }

    return AUDIO_OK;
}

static void Audio_I2sDeInit(AUDIO_Handle_t *dev) {
    const AUDIO_ConfigTypeDef *cfg = &dev->config;

    (void)HAL_I2S_DeInit(&dev->i2s);
    Audio_I2sClock(cfg->i2sInstance, false);

    Audio_I2sDeInitPin(&cfg->wsPin);
    Audio_I2sDeInitPin(&cfg->ckPin);
    Audio_I2sDeInitPin(&cfg->sdPin);
    Audio_I2sDeInitPin(&cfg->mclkPin);
}

static void Audio_I2sLinkDma(AUDIO_Handle_t *dev) {
    __HAL_LINKDMA(&dev->i2s, hdmatx, dev->dma);
}

static AUDIO_StatusTypeDef Audio_I2sStart(AUDIO_Handle_t *dev) {
    /* HAL_I2S_Transmit_DMA counts 16-bit words, hence half the byte count. */
    if (HAL_I2S_Transmit_DMA(&dev->i2s, (uint16_t *)(void *)dev->output.Buffer,
                             (uint16_t)(dev->output.Size / 2U)) != HAL_OK) {
        return AUDIO_ERROR;
    }
    return AUDIO_OK;
}

static void Audio_I2sStop(AUDIO_Handle_t *dev) {
    (void)HAL_I2S_DMAStop(&dev->i2s);
}

static void Audio_I2sPause(AUDIO_Handle_t *dev) {
    (void)HAL_I2S_DMAPause(&dev->i2s);
}

static void Audio_I2sResume(AUDIO_Handle_t *dev) {
    (void)HAL_I2S_DMAResume(&dev->i2s);
}

/* Public data ---------------------------------------------------------------*/

const AudioIfOps_t AudioI2sOps = {
    .name = "I2S",
    .init = Audio_I2sInit,
    .deinit = Audio_I2sDeInit,
    .linkDma = Audio_I2sLinkDma,
    .start = Audio_I2sStart,
    .stop = Audio_I2sStop,
    .pause = Audio_I2sPause,
    .resume = Audio_I2sResume,
};
