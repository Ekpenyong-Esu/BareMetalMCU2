/**
 * @file dma.h
 * @brief DMA driver aggregator
 *
 * Pulls in the public DMA modules:
 *  - dma_types.h    : data types and constants
 *  - dma_core.h     : initialization and lifecycle
 *  - dma_stream.h   : stream identification, clock gating and interrupt lines
 *  - dma_transfer.h : transfer control and status
 *  - dma_events.h   : interrupt dispatch and completion callbacks
 */

#ifndef DMA_H
#define DMA_H

#include "dma_types.h"
#include "dma_core.h"
#include "dma_stream.h"
#include "dma_transfer.h"
#include "dma_events.h"

#endif /* DMA_H */
