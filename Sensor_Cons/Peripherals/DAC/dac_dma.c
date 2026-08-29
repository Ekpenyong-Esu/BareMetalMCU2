/**
 * @file dac_dma.c
 * @brief Streams a sample buffer to the DAC over DMA
 */

#include "dac_dma.h"

#include "dac_core.h"
#include "log.h"

/* DAC channel 1 is hardwired to DMA1 Stream 5, Channel 7 on the STM32F4. */
#define DAC_DMA_STREAM      DMA1_Stream5
#define DAC_DMA_CHANNEL     DMA_CHANNEL_7
#define DAC_DMA_IRQN        DMA1_Stream5_IRQn

/* Numerically >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5) so the
 * completion callback may use FreeRTOS ...FromISR APIs. */
#define DAC_DMA_IRQ_PRIORITY  6U

static DMA_HandleTypeDef s_hdma;

static HAL_StatusTypeDef DacDmaInit(DAC_HandleStruct *hdac)
{
    __HAL_RCC_DMA1_CLK_ENABLE();

    s_hdma.Instance                 = DAC_DMA_STREAM;
    s_hdma.Init.Channel             = DAC_DMA_CHANNEL;
    s_hdma.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    s_hdma.Init.PeriphInc           = DMA_PINC_DISABLE;
    s_hdma.Init.MemInc              = DMA_MINC_ENABLE;
    /* Word on both sides: the sample buffer is uint32_t and DHR12R1 is 32-bit. */
    s_hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    s_hdma.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    s_hdma.Init.Mode                = DMA_CIRCULAR;
    s_hdma.Init.Priority            = DMA_PRIORITY_HIGH;
    s_hdma.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;

    if (HAL_DMA_Init(&s_hdma) != HAL_OK) {
        log_error("DAC: DMA init failed");
        return HAL_ERROR;
    }

    __HAL_LINKDMA(&hdac->hal_handle, DMA_Handle1, s_hdma);

    HAL_NVIC_SetPriority(DAC_DMA_IRQN, DAC_DMA_IRQ_PRIORITY, 0U);
    HAL_NVIC_EnableIRQ(DAC_DMA_IRQN);
    return HAL_OK;
}

HAL_StatusTypeDef DAC_StartCircularDMA(DAC_HandleStruct *hdac, uint32_t channel,
                                       uint32_t *samples, uint32_t length)
{
    if (!DAC_IsChannelValid(hdac, channel) || samples == NULL || length == 0U) {
        return HAL_ERROR;
    }

    if (s_hdma.Instance == NULL && DacDmaInit(hdac) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_DAC_Start_DMA(&hdac->hal_handle, channel, samples, length,
                             DAC_ALIGN_12B_R);
}

HAL_StatusTypeDef DAC_StopDMA(DAC_HandleStruct *hdac, uint32_t channel)
{
    if (!DAC_IsChannelValid(hdac, channel)) {
        return HAL_ERROR;
    }
    return HAL_DAC_Stop_DMA(&hdac->hal_handle, channel);
}

void DAC_DMA_IrqHandler(void)
{
    HAL_DMA_IRQHandler(&s_hdma);
}
