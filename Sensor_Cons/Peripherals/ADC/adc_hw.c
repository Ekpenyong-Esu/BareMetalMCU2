/**
 * @file    adc_hw.c
 * @brief   ADC hardware plumbing implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "adc_hw.h"

#include <stddef.h>

/* Private types -------------------------------------------------------------*/

typedef struct {
    ADC_TypeDef *instance;      /*!< ADC peripheral */
    DMA_Stream_TypeDef *stream; /*!< Stream wired to that ADC on DMA2 */
    uint32_t dmaChannel;        /*!< Request channel on that stream */
} ADC_InstanceMapEntry_t;

/* Private constants ---------------------------------------------------------*/

/** @brief Per-instance DMA routing, fixed by the STM32F429 request map */
static const ADC_InstanceMapEntry_t adc_instance_map[ADC_INSTANCE_COUNT] = {
    {ADC1, DMA2_Stream0, DMA_CHANNEL_0},
    {ADC2, DMA2_Stream2, DMA_CHANNEL_1},
    {ADC3, DMA2_Stream1, DMA_CHANNEL_2},
};

/* Private functions ---------------------------------------------------------*/

static const ADC_InstanceMapEntry_t *ADC_FindInstance(const ADC_TypeDef *instance) {
    for (uint32_t i = 0; i < ADC_INSTANCE_COUNT; i++) {
        if (adc_instance_map[i].instance == instance) {
            return &adc_instance_map[i];
        }
    }
    return NULL;
}

/* Exported functions --------------------------------------------------------*/

uint32_t ADC_InstanceIndex(const ADC_TypeDef *instance) {
    for (uint32_t i = 0; i < ADC_INSTANCE_COUNT; i++) {
        if (adc_instance_map[i].instance == instance) {
            return i;
        }
    }
    return ADC_INSTANCE_COUNT;
}

HAL_StatusTypeDef ADC_EnableInstanceClock(const ADC_TypeDef *instance) {
    if (instance == ADC1) {
        __HAL_RCC_ADC1_CLK_ENABLE();
    }
    else if (instance == ADC2) {
        __HAL_RCC_ADC2_CLK_ENABLE();
    }
    else if (instance == ADC3) {
        __HAL_RCC_ADC3_CLK_ENABLE();
    }
    else {
        return HAL_ERROR;
    }

    /* GPIO port clocks are enabled by the GPIO driver when pins are configured */
    return HAL_OK;
}

HAL_StatusTypeDef ADC_ConfigureDma(ADC_HandleStruct *hadc) {
    if (hadc == NULL) {
        return HAL_ERROR;
    }

    const ADC_InstanceMapEntry_t *entry = ADC_FindInstance(hadc->hal_handle.Instance);
    if (entry == NULL) {
        return HAL_ERROR;
    }

    __HAL_RCC_DMA2_CLK_ENABLE();

    hadc->hdma_adc.Instance = entry->stream;
    hadc->hdma_adc.Init.Channel = entry->dmaChannel;
    hadc->hdma_adc.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hadc->hdma_adc.Init.PeriphInc = DMA_PINC_DISABLE;
    hadc->hdma_adc.Init.MemInc = DMA_MINC_ENABLE;
    hadc->hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hadc->hdma_adc.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hadc->hdma_adc.Init.Mode = DMA_NORMAL;
    hadc->hdma_adc.Init.Priority = DMA_PRIORITY_HIGH;
    hadc->hdma_adc.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

    HAL_StatusTypeDef status = HAL_DMA_Init(&hadc->hdma_adc);
    if (status != HAL_OK) {
        return status;
    }

    __HAL_LINKDMA(&hadc->hal_handle, DMA_Handle, hadc->hdma_adc);

    return HAL_OK;
}

uint32_t ADC_ValidateResolution(uint32_t resolution) {
    switch (resolution) {
        case ADC_RESOLUTION_12B:
        case ADC_RESOLUTION_10B:
        case ADC_RESOLUTION_8B:
        case ADC_RESOLUTION_6B:
            return resolution;
        default:
            return ADC_RESOLUTION_12B;
    }
}

uint32_t ADC_ValidateSamplingTime(uint32_t sampling_time) {
    switch (sampling_time) {
        case ADC_SAMPLETIME_3CYCLES:
        case ADC_SAMPLETIME_15CYCLES:
        case ADC_SAMPLETIME_28CYCLES:
        case ADC_SAMPLETIME_56CYCLES:
        case ADC_SAMPLETIME_84CYCLES:
        case ADC_SAMPLETIME_112CYCLES:
        case ADC_SAMPLETIME_144CYCLES:
        case ADC_SAMPLETIME_480CYCLES:
            return sampling_time;
        default:
            return ADC_SAMPLETIME_84CYCLES;
    }
}
