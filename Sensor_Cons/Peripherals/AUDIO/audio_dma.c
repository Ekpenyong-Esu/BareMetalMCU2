/**
 ******************************************************************************
 * @file    audio_dma.c
 * @brief   Transmit DMA setup shared by the audio backends
 ******************************************************************************
 */

#include "audio_dma.h"
#include "dma_stream.h"

/* Private defines -----------------------------------------------------------*/

#define AUDIO_DMA_IRQ_PREEMPT_PRIORITY 0U
#define AUDIO_DMA_IRQ_SUB_PRIORITY 0U

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Enable the clock of the controller that owns a stream
 * @param   stream Stream belonging to either DMA1 or DMA2
 * @retval  None
 */
static void Audio_DmaEnableControllerClock(const DMA_Stream_TypeDef *stream) {
    if ((uint32_t)stream >= (uint32_t)DMA2_BASE) {
        __HAL_RCC_DMA2_CLK_ENABLE();
    }
    else {
        __HAL_RCC_DMA1_CLK_ENABLE();
    }
}

/* Public functions ----------------------------------------------------------*/

AUDIO_StatusTypeDef Audio_DmaInit(AUDIO_Handle_t *dev) {
    if (dev == NULL || dev->ops == NULL || dev->config.dmaStream == NULL) {
        return AUDIO_INVALID_PARAM;
    }

    /* Rejects addresses that are not one of the sixteen real streams. */
    if (!DMA_GetStreamIRQ(dev->config.dmaStream, &dev->dmaIrq)) {
        return AUDIO_INVALID_PARAM;
    }

    Audio_DmaEnableControllerClock(dev->config.dmaStream);

    dev->dma.Instance = dev->config.dmaStream;
    dev->dma.Init.Channel = dev->config.dmaChannel;
    dev->dma.Init.Direction = DMA_MEMORY_TO_PERIPH;
    dev->dma.Init.PeriphInc = DMA_PINC_DISABLE;
    dev->dma.Init.MemInc = DMA_MINC_ENABLE;
    dev->dma.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    dev->dma.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
    dev->dma.Init.Mode = DMA_CIRCULAR;
    dev->dma.Init.Priority = DMA_PRIORITY_HIGH;
    dev->dma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    dev->dma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    dev->dma.Init.MemBurst = DMA_MBURST_SINGLE;
    dev->dma.Init.PeriphBurst = DMA_PBURST_SINGLE;

    if (HAL_DMA_Init(&dev->dma) != HAL_OK) {
        dev->dma.Instance = NULL;
        return AUDIO_ERROR;
    }

    /* Only the backend knows which HAL handle the stream feeds. */
    dev->ops->linkDma(dev);

    HAL_NVIC_SetPriority(dev->dmaIrq, AUDIO_DMA_IRQ_PREEMPT_PRIORITY, AUDIO_DMA_IRQ_SUB_PRIORITY);
    HAL_NVIC_EnableIRQ(dev->dmaIrq);

    return AUDIO_OK;
}

void Audio_DmaDeInit(AUDIO_Handle_t *dev) {
    if (dev == NULL || dev->dma.Instance == NULL) {
        return;
    }

    HAL_NVIC_DisableIRQ(dev->dmaIrq);
    (void)HAL_DMA_DeInit(&dev->dma);
    dev->dma.Instance = NULL;
}
