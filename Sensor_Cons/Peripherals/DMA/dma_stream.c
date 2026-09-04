/**
 * @file dma_stream.c
 * @brief Stream identification, clock gating and interrupt line ownership
 */

#include "dma_stream.h"

typedef struct {
    const DMA_Stream_TypeDef *stream;
    IRQn_Type irqn;
    bool isController2;
} DMA_StreamEntry_t;

/* One entry per real stream, so an address that merely falls inside the
   controller's register block cannot be mistaken for a valid stream. */
static const DMA_StreamEntry_t s_streams[] = {
    {DMA1_Stream0, DMA1_Stream0_IRQn, false}, {DMA1_Stream1, DMA1_Stream1_IRQn, false},
    {DMA1_Stream2, DMA1_Stream2_IRQn, false}, {DMA1_Stream3, DMA1_Stream3_IRQn, false},
    {DMA1_Stream4, DMA1_Stream4_IRQn, false}, {DMA1_Stream5, DMA1_Stream5_IRQn, false},
    {DMA1_Stream6, DMA1_Stream6_IRQn, false}, {DMA1_Stream7, DMA1_Stream7_IRQn, false},
    {DMA2_Stream0, DMA2_Stream0_IRQn, true},  {DMA2_Stream1, DMA2_Stream1_IRQn, true},
    {DMA2_Stream2, DMA2_Stream2_IRQn, true},  {DMA2_Stream3, DMA2_Stream3_IRQn, true},
    {DMA2_Stream4, DMA2_Stream4_IRQn, true},  {DMA2_Stream5, DMA2_Stream5_IRQn, true},
    {DMA2_Stream6, DMA2_Stream6_IRQn, true},  {DMA2_Stream7, DMA2_Stream7_IRQn, true},
};

#define DMA_STREAM_COUNT (sizeof(s_streams) / sizeof(s_streams[0]))

static const DMA_StreamEntry_t *DMA_FindStream(const DMA_Stream_TypeDef *stream) {
    for (uint32_t i = 0U; i < DMA_STREAM_COUNT; i++) {
        if (s_streams[i].stream == stream) {
            return &s_streams[i];
        }
    }

    return NULL;
}

bool DMA_GetStreamIRQ(const DMA_Stream_TypeDef *stream, IRQn_Type *irqn) {
    const DMA_StreamEntry_t *entry = DMA_FindStream(stream);

    if ((entry == NULL) || (irqn == NULL)) {
        return false;
    }

    *irqn = entry->irqn;

    return true;
}

HAL_StatusTypeDef DMA_EnableClockAndIRQ(DMA_Handle_t *handle) {
    const DMA_StreamEntry_t *entry = NULL;

    if (handle == NULL) {
        return HAL_ERROR;
    }

    entry = DMA_FindStream(handle->config.stream);
    if (entry == NULL) {
        return HAL_ERROR;
    }

    if (entry->isController2) {
        __HAL_RCC_DMA2_CLK_ENABLE();
    }
    else {
        __HAL_RCC_DMA1_CLK_ENABLE();
    }

    handle->irqn = entry->irqn;

    HAL_NVIC_SetPriority(handle->irqn, DMA_IRQ_PRIORITY, DMA_IRQ_SUBPRIORITY);
    HAL_NVIC_EnableIRQ(handle->irqn);

    return HAL_OK;
}
