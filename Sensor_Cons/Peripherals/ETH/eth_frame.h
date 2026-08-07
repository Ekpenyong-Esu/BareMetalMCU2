/**
 * @file eth_frame.h
 * @brief Ethernet frame transmission and reception
 */

#ifndef ETH_FRAME_H
#define ETH_FRAME_H

#ifdef __cplusplus
extern "C" {
#endif

#include "eth_types.h"

/** @brief Length of the frame check sequence appended by the MAC */
#define ETH_FCS_LEN     4U

/**
 * @brief   Transmit a single Ethernet frame
 * @param   handle Driver handle
 * @param   frame Frame to send, payload no longer than ETH_MAX_PAYLOAD_LEN
 * @retval  HAL_StatusTypeDef HAL_OK on success
 * @note    The MAC inserts the pad and the CRC, so short payloads are padded
 *          to the minimum frame length in hardware.
 */
HAL_StatusTypeDef ETH_TransmitFrame(ETH_Handle_t *handle, const ETH_Frame_t *frame);

/**
 * @brief   Fetch one received Ethernet frame if the DMA has one ready
 * @param   handle Driver handle
 * @param   frame Destination for the parsed frame
 * @retval  HAL_StatusTypeDef HAL_OK when a frame was returned
 * @note    frame->payload points into a driver owned receive buffer and stays
 *          valid only until ETH_RX_DESC_CNT further frames have been received.
 */
HAL_StatusTypeDef ETH_ReceiveFrame(ETH_Handle_t *handle, ETH_Frame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* ETH_FRAME_H */
