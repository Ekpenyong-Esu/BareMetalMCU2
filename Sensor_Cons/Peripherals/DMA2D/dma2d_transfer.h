/**
 * @file dma2d_transfer.h
 * @brief DMA2D transfer operations (polling and interrupt driven)
 *
 * @details
 * Every entry point here shares the same three steps: validate, guard against a
 * busy or uninitialised peripheral, then launch. The polling and interrupt
 * variants differ only in which HAL start function is used, so they are built
 * from a single implementation.
 */

#ifndef DMA2D_TRANSFER_H
#define DMA2D_TRANSFER_H

#include "dma2d_types.h"

/**
 * @brief Start a transfer in the configured mode and wait for it (polling)
 * @param pSrc Source buffer, required for every mode except register-to-memory
 * @param pDst Destination buffer (must not be NULL)
 * @param width Transfer width in pixels (1 to DMA2D_MAX_WIDTH)
 * @param height Transfer height in pixels (1 to DMA2D_MAX_HEIGHT)
 * @return HAL_OK on completion, HAL_BUSY, or HAL_ERROR/HAL_TIMEOUT on failure
 */
HAL_StatusTypeDef DMA2D_StartTransfer(const uint32_t *pSrc, uint32_t *pDst, uint32_t width,
                                      uint32_t height);

/**
 * @brief Fill an area with a solid colour and wait for it (polling)
 * @param color Fill color value (ARGB8888 format)
 * @param pDst Destination buffer (must not be NULL)
 * @param width Fill width in pixels
 * @param height Fill height in pixels
 * @return HAL_OK on completion, HAL_BUSY, or HAL_ERROR/HAL_TIMEOUT on failure
 */
HAL_StatusTypeDef DMA2D_StartFill(uint32_t color, uint32_t *pDst, uint32_t width, uint32_t height);

/**
 * @brief Blend two ARGB8888 buffers and wait for it (polling)
 * @param pSrc1 Foreground buffer (must not be NULL)
 * @param pSrc2 Background buffer (must not be NULL)
 * @param pDst Destination buffer (must not be NULL)
 * @param width Blend width in pixels
 * @param height Blend height in pixels
 * @return HAL_OK on completion, HAL_BUSY, or HAL_ERROR/HAL_TIMEOUT on failure
 */
HAL_StatusTypeDef DMA2D_StartBlending(const uint32_t *pSrc1, const uint32_t *pSrc2, uint32_t *pDst,
                                      uint32_t width, uint32_t height);

/**
 * @brief Start a transfer in the configured mode and return immediately
 * @param pSrc Source buffer, required for every mode except register-to-memory
 * @param pDst Destination buffer (must not be NULL)
 * @param width Transfer width in pixels
 * @param height Transfer height in pixels
 * @return HAL_OK when the transfer was started, HAL_BUSY or HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_StartTransfer_IT(const uint32_t *pSrc, uint32_t *pDst, uint32_t width,
                                         uint32_t height);

/**
 * @brief Start a solid colour fill and return immediately
 * @param color Fill color value (ARGB8888 format)
 * @param pDst Destination buffer (must not be NULL)
 * @param width Fill width in pixels
 * @param height Fill height in pixels
 * @return HAL_OK when the fill was started, HAL_BUSY or HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_StartFill_IT(uint32_t color, uint32_t *pDst, uint32_t width,
                                     uint32_t height);

/**
 * @brief Start a blend of two ARGB8888 buffers and return immediately
 * @param pSrc1 Foreground buffer (must not be NULL)
 * @param pSrc2 Background buffer (must not be NULL)
 * @param pDst Destination buffer (must not be NULL)
 * @param width Blend width in pixels
 * @param height Blend height in pixels
 * @return HAL_OK when the blend was started, HAL_BUSY or HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_StartBlending_IT(const uint32_t *pSrc1, const uint32_t *pSrc2,
                                         uint32_t *pDst, uint32_t width, uint32_t height);

/**
 * @brief Fill a small buffer and read it back to prove the unit works
 * @return HAL_OK when the readback matches, HAL_ERROR otherwise
 */
HAL_StatusTypeDef DMA2D_SelfTest(void);

#endif /* DMA2D_TRANSFER_H */
