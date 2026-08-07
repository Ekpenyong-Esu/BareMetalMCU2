/**
  ******************************************************************************
  * @file    mic_hw.c
  * @brief   Board wiring and transport bring-up for the PDM microphone
  ******************************************************************************
  */

#include "mic_hw.h"
#include "gpio.h"

MIC_StatusTypeDef MIC_HW_InitGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO driver enables the port clock */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = MIC_CLK_PIN;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    GPIO_Driver_Pin_Init(MIC_CLK_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MIC_DATA_PIN;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    GPIO_Driver_Pin_Init(MIC_DATA_GPIO_PORT, &GPIO_InitStruct);

    return MIC_OK;
}

MIC_StatusTypeDef MIC_HW_InitI2S(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL || hmic->hi2s == NULL) {
        return MIC_INVALID_PARAM;
    }

    MIC_I2S_CLK_ENABLE();

    hmic->hi2s->Instance = MIC_I2S;
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

MIC_StatusTypeDef MIC_HW_InitDMA(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL || hmic->hdma == NULL || hmic->hi2s == NULL) {
        return MIC_INVALID_PARAM;
    }

    MIC_DMA_CLK_ENABLE();

    hmic->hdma->Instance = MIC_DMA_STREAM;
    hmic->hdma->Init.Channel = MIC_DMA_CHANNEL;
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

MIC_StatusTypeDef MIC_HW_SetSampleRate(MIC_HandleTypeDef *hmic, uint32_t sampleRate)
{
    if (hmic == NULL || hmic->hi2s == NULL) {
        return MIC_INVALID_PARAM;
    }

    hmic->hi2s->Init.AudioFreq = sampleRate;
    if (HAL_I2S_Init(hmic->hi2s) != HAL_OK) {
        return MIC_I2S_ERROR;
    }

    return MIC_OK;
}
