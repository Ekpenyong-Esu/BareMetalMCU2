/**
  ******************************************************************************
  * @file    can_transfer.h
  * @brief   CAN frame transmit and receive
  * @details Blocking and non-blocking data path for one CAN handle. Wraps
  *          HAL_CAN_AddTxMessage / HAL_CAN_GetRxMessage with frame conversion,
  *          timeout handling and callback dispatch.
  *
  * CAN Transfer Responsibilities:
  * - CAN_Transmit/Receive: Frame conversion, HAL calls, timeout/abort, callbacks
  * - Mailbox/FIFO queries: Free level, pending count, per-FIFO checks
  * - Abort: Cancel a queued transmit by mailbox index
  *
  * Transmit Flow:
  * 1. Convert CAN_Frame -> CAN_TxHeaderTypeDef (StdId/ExtId, IDE, RTR, DLC)
  * 2. HAL_CAN_AddTxMessage() — picks a free mailbox
  * 3. If timeout > 0, poll HAL_CAN_IsTxMessagePending() until done or timeout
  * 4. On timeout, abort the mailbox so it does not stay occupied
  * 5. Bump tx_count, return
  *
  * Receive Flow:
  * 1. If no FIFO has data and timeout == 0, return immediately
  * 2. Otherwise poll until a FIFO has data or timeout expires
  * 3. HAL_CAN_GetRxMessage() from the non-empty FIFO
  * 4. Convert CAN_RxHeaderTypeDef -> CAN_Frame, bump rx_count, fire callback
  */

#ifndef CAN_TRANSFER_H
#define CAN_TRANSFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

/**
 * @brief   Queue a frame for transmission
 *
 * Converts the driver frame to a HAL Tx header, submits it to a free
 * mailbox, and optionally waits for completion. On timeout the mailbox
 * is aborted so the caller does not leave it occupied.
 *
 * @param   hcan Handle (must be initialized)
 * @param   frame Frame to send (data_length 0..8)
 * @param   timeout Max wait in ms; 0 = fire-and-forget
 * @retval  HAL_StatusTypeDef HAL_OK, HAL_ERROR or HAL_TIMEOUT
 */
HAL_StatusTypeDef CAN_Transmit(CAN_Handle_t *hcan, const CAN_Frame *frame, uint32_t timeout);

/**
 * @brief   Receive the next pending frame
 *
 * Checks both FIFOs; if neither has data and timeout == 0 returns
 * immediately, otherwise polls until data arrives or timeout expires.
 * Converts the HAL Rx header to a driver frame and fires the Rx callback.
 *
 * @param   hcan Handle (must be initialized)
 * @param   frame Output frame
 * @param   timeout Max wait in ms; 0 = non-blocking poll
 * @retval  HAL_StatusTypeDef HAL_OK, HAL_ERROR or HAL_TIMEOUT
 */
HAL_StatusTypeDef CAN_Receive(CAN_Handle_t *hcan, CAN_Frame *frame, uint32_t timeout);

/**
 * @brief   Check whether any transmit mailbox is free
 * @param   hcan Handle
 * @retval  bool true when at least one mailbox is free
 */
bool CAN_IsTransmitMailboxAvailable(CAN_Handle_t *hcan);

/**
 * @brief   Check whether a receive FIFO has pending frames
 * @param   hcan Handle
 * @param   fifo_number FIFO index (0 or 1)
 * @retval  bool true when at least one frame is waiting
 */
bool CAN_IsReceivePending(CAN_Handle_t *hcan, uint8_t fifo_number);

/**
 * @brief   Count pending frames in a receive FIFO
 * @param   hcan Handle
 * @param   fifo_number FIFO index (0 or 1)
 * @retval  uint8_t Number of frames waiting (0 if invalid FIFO)
 */
uint8_t CAN_GetReceivePendingCount(CAN_Handle_t *hcan, uint8_t fifo_number);

/**
 * @brief   Abort a queued transmit
 * @param   hcan Handle
 * @param   mailbox_number Mailbox index (0..2)
 * @retval  HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid mailbox
 */
HAL_StatusTypeDef CAN_AbortTransmit(CAN_Handle_t *hcan, uint8_t mailbox_number);

#ifdef __cplusplus
}
#endif

#endif /* CAN_TRANSFER_H */
