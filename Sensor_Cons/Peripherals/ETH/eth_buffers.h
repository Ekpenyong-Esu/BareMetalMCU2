/**
 * @file eth_buffers.h
 * @brief Internal DMA descriptor and packet buffer ownership
 * @note  Not part of the public eth.h aggregator.
 */

#ifndef ETH_BUFFERS_H
#define ETH_BUFFERS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eth_types.h"

/** @brief Buffer size handed to the DMA, a multiple of 4 as the MAC requires */
#define ETH_BUFFER_SIZE     ETH_RX_BUF_SIZE

/**
 * @brief   Attach the descriptor tables to a HAL handle before HAL_ETH_Init()
 * @param   heth HAL handle being prepared
 */
void ETH_Buffers_Attach(ETH_HandleTypeDef *heth);

/**
 * @brief   Reset the receive buffer rotation
 */
void ETH_Buffers_Reset(void);

/**
 * @brief   Access the single transmit staging buffer
 * @retval  uint8_t* Buffer of ETH_BUFFER_SIZE bytes
 */
uint8_t *ETH_Buffers_GetTxBuffer(void);

#ifdef __cplusplus
}
#endif

#endif /* ETH_BUFFERS_H */
