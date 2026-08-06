/**
 * @file uart_dma.h
 * @brief DMA transfer mode
 *
 * Buffer transfers are offloaded to a DMA stream; the streams themselves are
 * wired up by HAL_UART_MspInit() in Core.
 */

#ifndef UART_DMA_H
#define UART_DMA_H

#ifdef __cplusplus
extern "C" {
#endif

#include "uart_types.h"

extern const UART_ModeOps_t UART_DmaOps;

#ifdef __cplusplus
}
#endif

#endif /* UART_DMA_H */
