/**
 ******************************************************************************
 * @file    mic_hw.c
 * @brief   Transport bring-up for the PDM microphone
 ******************************************************************************
 */

#include "mic_hw.h"
#include "gpio.h"

/* ---- Instance facts: clock gates and alternate functions ----------------- */

/* Leaving Alternate at 0 would map the pin to SYS, not to the I2S block. */
static bool MIC_HW_ResolveAlternate(const SPI_TypeDef *instance, uint8_t *alternate) {
    if (instance == SPI2) {
        *alternate = GPIO_AF5_SPI2;
    }
    else if (instance == SPI3) {
        *alternate = GPIO_AF6_SPI3;
    }
    else {
        return false;
    }

    return true;
}

static bool MIC_HW_I2SClockEnable(const SPI_TypeDef *instance) {
    if (instance == SPI2) {
        __HAL_RCC_SPI2_CLK_ENABLE();
    }
    else if (instance == SPI3) {
        __HAL_RCC_SPI3_CLK_ENABLE();
    }
    else {
        return false;
    }

    return true;
}

static void MIC_HW_I2SClockDisable(const SPI_TypeDef *instance) {
    if (instance == SPI2) {
        __HAL_RCC_SPI2_CLK_DISABLE();
    }
    else if (instance == SPI3) {
        __HAL_RCC_SPI3_CLK_DISABLE();
    }
}

static bool MIC_HW_IsDMA1Stream(const DMA_Stream_TypeDef *stream) {
    const uintptr_t address = (uintptr_t)stream;

    return (address >= (uintptr_t)DMA1_Stream0) && (address <= (uintptr_t)DMA1_Stream7);
}

static bool MIC_HW_IsDMA2Stream(const DMA_Stream_TypeDef *stream) {
    const uintptr_t address = (uintptr_t)stream;

    return (address >= (uintptr_t)DMA2_Stream0) && (address <= (uintptr_t)DMA2_Stream7);
}

static bool MIC_HW_DMAClockEnable(const DMA_Stream_TypeDef *stream) {
    if (MIC_HW_IsDMA1Stream(stream)) {
        __HAL_RCC_DMA1_CLK_ENABLE();
    }
    else if (MIC_HW_IsDMA2Stream(stream)) {
        __HAL_RCC_DMA2_CLK_ENABLE();
    }
    else {
        return false;
    }

    return true;
}

/* ---- Wiring -------------------------------------------------------------- */

bool MIC_HW_IsValidWiring(const MIC_ConfigTypeDef *config) {
    uint8_t alternate = 0U;

    if (config == NULL || config->clkPort == NULL || config->dataPort == NULL ||
        config->clkPin == 0U || config->dataPin == 0U) {
        return false;
    }

    if (!MIC_HW_ResolveAlternate(config->i2sInstance, &alternate)) {
        return false;
    }

    return MIC_HW_IsDMA1Stream(config->dmaStream) || MIC_HW_IsDMA2Stream(config->dmaStream);
}

static void MIC_HW_PinInit(GPIO_TypeDef *port, uint16_t pin, uint8_t alternate) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO driver enables the port clock */
    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = alternate;
    GPIO_Driver_Pin_Init(port, &GPIO_InitStruct);
}

MIC_StatusTypeDef MIC_HW_InitGPIO(const MIC_ConfigTypeDef *config) {
    uint8_t alternate = 0U;

    if (!MIC_HW_IsValidWiring(config)) {
        return MIC_INVALID_PARAM;
    }

    if (config->alternate != 0U) {
        alternate = config->alternate;
    }
    else {
        (void)MIC_HW_ResolveAlternate(config->i2sInstance, &alternate);
    }

    MIC_HW_PinInit(config->clkPort, config->clkPin, alternate);
    MIC_HW_PinInit(config->dataPort, config->dataPin, alternate);

    return MIC_OK;
}

static void MIC_HW_DeInitGPIO(const MIC_ConfigTypeDef *config) {
    if (config->clkPort != NULL) {
        GPIO_Driver_Pin_DeInit(config->clkPort, config->clkPin);
    }
    if (config->dataPort != NULL) {
        GPIO_Driver_Pin_DeInit(config->dataPort, config->dataPin);
    }
}

/* ---- I2S and DMA --------------------------------------------------------- */

MIC_StatusTypeDef MIC_HW_InitI2S(MIC_HandleTypeDef *hmic) {
    if (hmic == NULL || hmic->hi2s == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!MIC_HW_I2SClockEnable(hmic->Config.i2sInstance)) {
        return MIC_INVALID_PARAM;
    }

    hmic->hi2s->Instance = hmic->Config.i2sInstance;
    hmic->hi2s->Init.Mode = I2S_MODE_MASTER_RX;
    hmic->hi2s->Init.Standard = I2S_STANDARD_LSB;
    hmic->hi2s->Init.DataFormat = I2S_DATAFORMAT_16B;
    hmic->hi2s->Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    hmic->hi2s->Init.AudioFreq = MIC_DEFAULT_SAMPLE_RATE;
    hmic->hi2s->Init.CPOL = I2S_CPOL_HIGH;
    hmic->hi2s->Init.ClockSource = I2S_CLOCK_PLL;
    hmic->hi2s->Init.FullDuplexMode = I2S_FULLDUPLEXMODE_DISABLE;

    if (HAL_I2S_Init(hmic->hi2s) != HAL_OK) {
        return MIC_I2S_ERROR;
    }

    return MIC_OK;
}

MIC_StatusTypeDef MIC_HW_InitDMA(MIC_HandleTypeDef *hmic) {
    if (hmic == NULL || hmic->hdma == NULL || hmic->hi2s == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!MIC_HW_DMAClockEnable(hmic->Config.dmaStream)) {
        return MIC_INVALID_PARAM;
    }

    hmic->hdma->Instance = hmic->Config.dmaStream;
    hmic->hdma->Init.Channel = hmic->Config.dmaChannel;
    hmic->hdma->Init.Direction = DMA_PERIPH_TO_MEMORY;
    hmic->hdma->Init.PeriphInc = DMA_PINC_DISABLE;
    hmic->hdma->Init.MemInc = DMA_MINC_ENABLE;
    hmic->hdma->Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hmic->hdma->Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    hmic->hdma->Init.Mode = DMA_CIRCULAR;
    hmic->hdma->Init.Priority = DMA_PRIORITY_HIGH;
    hmic->hdma->Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(hmic->hdma) != HAL_OK) {
        return MIC_DMA_ERROR;
    }

    __HAL_LINKDMA(hmic->hi2s, hdmarx, *hmic->hdma);

    return MIC_OK;
}

void MIC_HW_DeInit(MIC_HandleTypeDef *hmic) {
    if (hmic == NULL) {
        return;
    }

    if (hmic->hdma != NULL) {
        (void)HAL_DMA_DeInit(hmic->hdma);
    }

    if (hmic->hi2s != NULL) {
        (void)HAL_I2S_DeInit(hmic->hi2s);
        MIC_HW_I2SClockDisable(hmic->Config.i2sInstance);
    }

    MIC_HW_DeInitGPIO(&hmic->Config);
}

MIC_StatusTypeDef MIC_HW_SetSampleRate(MIC_HandleTypeDef *hmic, uint32_t sampleRate) {
    if (hmic == NULL || hmic->hi2s == NULL) {
        return MIC_INVALID_PARAM;
    }

    hmic->hi2s->Init.AudioFreq = sampleRate;
    if (HAL_I2S_Init(hmic->hi2s) != HAL_OK) {
        return MIC_I2S_ERROR;
    }

    return MIC_OK;
}
