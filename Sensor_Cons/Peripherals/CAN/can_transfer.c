/**
  ******************************************************************************
  * @file    can_transfer.c
  * @brief   CAN frame transmit and receive implementation
  *
  * This module implements the CAN data path:
  * - Frame conversion between driver CAN_Frame and HAL Tx/Rx headers
  * - Transmit with optional blocking wait and timeout abort
  * - Receive with FIFO selection, polling and callback dispatch
  * - Mailbox and FIFO status queries
  *
  * Key Design Points:
  * - CAN_FillFrameFromRxHeader() centralizes Rx header -> frame conversion
  * - Transmit aborts the mailbox on timeout so it does not stay occupied
  * - Receive checks both FIFOs and prefers FIFO0 when both have data
  * - All functions are handle-based; no file-scope state
  */

#include "can_transfer.h"
#include "can_core.h"
#include "can_events.h"

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Convert a HAL Rx header into a driver frame
 *
 * Copies id, frame type, DLC, RTR and timestamp, then zeroes any payload
 * bytes beyond DLC so the caller cannot read stale buffer contents.
 *
 * @param frame Output driver frame
 * @param rx_header HAL Rx header from HAL_CAN_GetRxMessage()
 */
static void CAN_FillFrameFromRxHeader(CAN_Frame *frame, const CAN_RxHeaderTypeDef *rx_header)
{
    frame->id = (rx_header->IDE == CAN_ID_STD) ? rx_header->StdId : rx_header->ExtId;
    frame->frame_type = (rx_header->IDE == CAN_ID_STD) ? CAN_FRAME_STANDARD : CAN_FRAME_EXTENDED;
    frame->data_length = rx_header->DLC;
    frame->remote_request = (rx_header->RTR == CAN_RTR_REMOTE);
    frame->timestamp = rx_header->Timestamp;

    /* HAL only writes DLC bytes; clear the rest so the caller cannot read
       whatever the buffer held before. */
    for (uint8_t i = frame->data_length; i < CAN_MAX_DATA_LENGTH; i++) {
        frame->data[i] = 0U;
    }
}

/* Exported functions --------------------------------------------------------*/

/**
 * @brief Queue a frame for transmission
 * @param hcan Handle (must be initialized)
 * @param frame Frame to send (data_length 0..8)
 * @param timeout Max wait in ms; 0 = fire-and-forget
 * @retval HAL_StatusTypeDef HAL_OK, HAL_ERROR or HAL_TIMEOUT
 */

HAL_StatusTypeDef CAN_Transmit(CAN_Handle_t *hcan, const CAN_Frame *frame, uint32_t timeout)
{
    CAN_TxHeaderTypeDef tx_header = {0};
    uint32_t tx_mailbox = 0;

    if (hcan == NULL || frame == NULL || frame->data_length > CAN_MAX_DATA_LENGTH) {
        return HAL_ERROR;
    }

    tx_header.StdId = (frame->frame_type == CAN_FRAME_STANDARD) ? frame->id : 0;
    tx_header.ExtId = (frame->frame_type == CAN_FRAME_EXTENDED) ? frame->id : 0;
    tx_header.IDE = (frame->frame_type == CAN_FRAME_STANDARD) ? CAN_ID_STD : CAN_ID_EXT;
    tx_header.RTR = frame->remote_request ? CAN_RTR_REMOTE : CAN_RTR_DATA;
    tx_header.DLC = frame->data_length;
    tx_header.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(&hcan->hal, &tx_header, (uint8_t *)(uintptr_t)frame->data,
                             &tx_mailbox) != HAL_OK) {
        return HAL_ERROR;
    }

    if (timeout > 0U) {
        uint32_t start = HAL_GetTick();
        while (HAL_CAN_IsTxMessagePending(&hcan->hal, tx_mailbox)) {
            if ((HAL_GetTick() - start) > timeout) {
                /* Drop the request instead of leaving the mailbox occupied by
                   a frame the caller has already given up on. */
                (void)HAL_CAN_AbortTxRequest(&hcan->hal, tx_mailbox);
                return HAL_TIMEOUT;
            }
        }
    }

    CAN_IncrementTxCount(hcan);

    return HAL_OK;
}

/**
 * @brief Receive the next pending frame
 * @param hcan Handle (must be initialized)
 * @param frame Output frame
 * @param timeout Max wait in ms; 0 = non-blocking poll
 * @retval HAL_StatusTypeDef HAL_OK, HAL_ERROR or HAL_TIMEOUT
 */
HAL_StatusTypeDef CAN_Receive(CAN_Handle_t *hcan, CAN_Frame *frame, uint32_t timeout)
{
    CAN_RxHeaderTypeDef rx_header;

    if (hcan == NULL || frame == NULL) {
        return HAL_ERROR;
    }

    if (!CAN_IsReceivePending(hcan, 0) && !CAN_IsReceivePending(hcan, 1)) {
        if (timeout == 0U) {
            return HAL_ERROR;
        }
        uint32_t start = HAL_GetTick();
        while (!CAN_IsReceivePending(hcan, 0) && !CAN_IsReceivePending(hcan, 1)) {
            if ((HAL_GetTick() - start) > timeout) {
                return HAL_TIMEOUT;
            }
        }
    }

    uint32_t fifo = CAN_IsReceivePending(hcan, 0) ? CAN_RX_FIFO0 : CAN_RX_FIFO1;

    if (HAL_CAN_GetRxMessage(&hcan->hal, fifo, &rx_header, frame->data) != HAL_OK) {
        return HAL_ERROR;
    }

    CAN_FillFrameFromRxHeader(frame, &rx_header);
    CAN_IncrementRxCount(hcan);
    CAN_NotifyRx(hcan, frame);

    return HAL_OK;
}

/**
 * @brief Check whether any transmit mailbox is free
 * @param hcan Handle
 * @retval bool true when at least one mailbox is free
 */
bool CAN_IsTransmitMailboxAvailable(CAN_Handle_t *hcan)
{
    if (hcan == NULL) {
        return false;
    }

    return (HAL_CAN_GetTxMailboxesFreeLevel(&hcan->hal) > 0);
}

/**
 * @brief Check whether a receive FIFO has pending frames
 * @param hcan Handle
 * @param fifo_number FIFO index (0 or 1)
 * @retval bool true when at least one frame is waiting
 */
bool CAN_IsReceivePending(CAN_Handle_t *hcan, uint8_t fifo_number)
{
    uint32_t fifo = 0U;

    if (hcan == NULL || fifo_number > 1U) {
        return false;
    }

    fifo = (fifo_number == 0U) ? CAN_RX_FIFO0 : CAN_RX_FIFO1;
    return (HAL_CAN_GetRxFifoFillLevel(&hcan->hal, fifo) > 0);
}

/**
 * @brief Count pending frames in a receive FIFO
 * @param hcan Handle
 * @param fifo_number FIFO index (0 or 1)
 * @retval uint8_t Number of frames waiting (0 if invalid FIFO)
 */
uint8_t CAN_GetReceivePendingCount(CAN_Handle_t *hcan, uint8_t fifo_number)
{
    uint32_t fifo = 0U;

    if (hcan == NULL || fifo_number > 1U) {
        return 0;
    }

    fifo = (fifo_number == 0U) ? CAN_RX_FIFO0 : CAN_RX_FIFO1;
    return HAL_CAN_GetRxFifoFillLevel(&hcan->hal, fifo);
}

/**
 * @brief Abort a queued transmit
 * @param hcan Handle
 * @param mailbox_number Mailbox index (0..2)
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on invalid mailbox
 */
HAL_StatusTypeDef CAN_AbortTransmit(CAN_Handle_t *hcan, uint8_t mailbox_number)
{
    uint32_t mailbox = 0U;

    if (hcan == NULL || mailbox_number >= CAN_MAX_MAILBOXES) {
        return HAL_ERROR;
    }

    mailbox = (mailbox_number == 0U) ? CAN_TX_MAILBOX0 :
              (mailbox_number == 1U) ? CAN_TX_MAILBOX1 : CAN_TX_MAILBOX2;

    return HAL_CAN_AbortTxRequest(&hcan->hal, mailbox);
}
