/**
 * @file dma_transfer.h
 * @brief DMA transfer control and status
 */

#ifndef DMA_TRANSFER_H
#define DMA_TRANSFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dma_types.h"

/**
 * @brief   Start an interrupt driven transfer
 * @param   handle Handle
 * @param   srcAddr Source address
 * @param   dstAddr Destination address
 * @param   dataLength Number of items, at most DMA_MAX_TRANSFER_ITEMS
 * @retval  HAL_StatusTypeDef HAL_OK on success
 * @note    Both addresses must be aligned to the configured item width.
 */
HAL_StatusTypeDef DMA_StartTransfer(DMA_Handle_t *handle, uint32_t srcAddr,
                                    uint32_t dstAddr, uint32_t dataLength);

/**
 * @brief   Start a peripheral to memory transfer
 * @param   handle Handle configured with DMA_PERIPH_TO_MEMORY
 * @param   periphAddr Peripheral address
 * @param   memAddr Memory address
 * @param   dataLength Number of items
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DMA_StartPeriphToMem(DMA_Handle_t *handle, uint32_t periphAddr,
                                       uint32_t memAddr, uint32_t dataLength);

/**
 * @brief   Start a memory to peripheral transfer
 * @param   handle Handle configured with DMA_MEMORY_TO_PERIPH
 * @param   memAddr Memory address
 * @param   periphAddr Peripheral address
 * @param   dataLength Number of items
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DMA_StartMemToPeriph(DMA_Handle_t *handle, uint32_t memAddr,
                                       uint32_t periphAddr, uint32_t dataLength);

/**
 * @brief   Abort the transfer in progress
 * @param   handle Handle
 * @retval  HAL_StatusTypeDef HAL_OK on success
 */
HAL_StatusTypeDef DMA_StopTransfer(DMA_Handle_t *handle);

/**
 * @brief   Report whether a started transfer has finished
 * @param   handle Handle
 * @retval  bool true once the transfer that was started has completed
 * @note    Returns false when no transfer has ever been started, unlike a bare
 *          HAL state test, which reports READY before the first transfer.
 */
bool DMA_IsTransferComplete(const DMA_Handle_t *handle);

/**
 * @brief   Report whether a transfer is in flight
 * @param   handle Handle
 * @retval  bool true while a transfer is running
 */
bool DMA_IsBusy(const DMA_Handle_t *handle);

/**
 * @brief   Read the HAL error code of the last transfer
 * @param   handle Handle
 * @retval  uint32_t HAL_DMA_ERROR_* code; HAL_DMA_ERROR_NONE for an unusable handle
 */
uint32_t DMA_GetError(const DMA_Handle_t *handle);

#ifdef __cplusplus
}
#endif

#endif /* DMA_TRANSFER_H */
