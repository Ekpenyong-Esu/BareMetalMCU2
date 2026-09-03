/**
  ******************************************************************************
  * @file    can_events.c
  * @brief   CAN callback registration and interrupt management implementation
  * @details Per-handle callback storage and HAL ISR dispatch. Each handle
  *          owns its own tx/rx/error callbacks; the HAL ISR callbacks
  *          resolve the handle via the registry in can_core.c.
  *
  * This module implements per-handle callback storage and HAL ISR dispatch:
  * - Callback registration: CAN_Register*Callback store per-handle pointers
  * - Notification dispatch: CAN_Notify* invoke the stored callbacks
  * - Interrupt control: CAN_Enable/DisableInterrupts manage bxCAN notifications
  * - HAL callbacks: resolve the driver handle via the registry and dispatch
  ******************************************************************************
  */

#include "can_events.h"
#include "can_core.h"

/* Callback registration -----------------------------------------------------*/

/**
 * @brief Register a transmit-complete callback for a handle
 * @param hcan Handle
 * @param callback Function to call on mailbox complete (or NULL to clear)
 */
void CAN_RegisterTxCallback(CAN_Handle_t *hcan, CAN_TxCallback callback)
{
    if (hcan != NULL) {
        hcan->tx_callback = callback;
    }
}

/**
 * @brief Register a receive callback for a handle
 * @param hcan Handle
 * @param callback Function to call when a frame arrives (or NULL to clear)
 */
void CAN_RegisterRxCallback(CAN_Handle_t *hcan, CAN_RxCallback callback)
{
    if (hcan != NULL) {
        hcan->rx_callback = callback;
    }
}

/**
 * @brief Register an error callback for a handle
 * @param hcan Handle
 * @param callback Function to call on bus error (or NULL to clear)
 */
void CAN_RegisterErrorCallback(CAN_Handle_t *hcan, CAN_ErrorCallbackFn callback)
{
    if (hcan != NULL) {
        hcan->error_callback = callback;
    }
}

/* Notification dispatch -----------------------------------------------------*/

/**
 * @brief Dispatch the Rx callback for a handle
 * @param hcan Handle
 * @param frame Received frame
 */
void CAN_NotifyRx(CAN_Handle_t *hcan, const CAN_Frame *frame)
{
    if (hcan != NULL && hcan->rx_callback != NULL) {
        hcan->rx_callback(hcan, frame);
    }
}

/**
 * @brief Dispatch the Tx-complete callback for a handle
 * @param hcan Handle
 * @param mailbox Mailbox index (0..2) that completed
 */
void CAN_NotifyTxComplete(CAN_Handle_t *hcan, uint8_t mailbox)
{
    if (hcan != NULL && hcan->tx_callback != NULL) {
        hcan->tx_callback(hcan, mailbox);
    }
}

/**
 * @brief Classify and dispatch the error callback for a handle
 * @param hcan Handle
 */
void CAN_NotifyError(CAN_Handle_t *hcan)
{
    CAN_ErrorType error = CAN_GetErrorType(hcan);

    if (error != CAN_ERROR_NONE) {
        CAN_RecordError(hcan, error);
        if (hcan->error_callback != NULL) {
            hcan->error_callback(hcan, error);
        }
    }
}

/* Interrupt control ---------------------------------------------------------*/

/**
 * @brief Enable bxCAN interrupts for a handle
 * @param hcan Handle (must be initialized)
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_EnableInterrupts(CAN_Handle_t *hcan)
{
    static const uint32_t notifications[] = {
        CAN_IT_TX_MAILBOX_EMPTY,
        CAN_IT_RX_FIFO0_MSG_PENDING,
        CAN_IT_RX_FIFO1_MSG_PENDING,
        CAN_IT_RX_FIFO0_OVERRUN,
        CAN_IT_RX_FIFO1_OVERRUN,
        CAN_IT_ERROR,
        CAN_IT_BUSOFF,
        CAN_IT_ERROR_PASSIVE,
        CAN_IT_ERROR_WARNING,
    };

    if (hcan == NULL) {
        return HAL_ERROR;
    }

    for (size_t i = 0; i < (sizeof(notifications) / sizeof(notifications[0])); i++) {
        HAL_StatusTypeDef status = HAL_CAN_ActivateNotification(&hcan->hal, notifications[i]);

        if (status != HAL_OK) {
            return status;
        }
    }

    return HAL_OK;
}

/**
 * @brief Disable bxCAN interrupts for a handle
 * @param hcan Handle
 * @retval HAL_StatusTypeDef HAL_OK on success, HAL_ERROR on failure
 */
HAL_StatusTypeDef CAN_DisableInterrupts(CAN_Handle_t *hcan)
{
    /* The HAL takes a mask, so one call reports one result; deactivating them
       one at a time only made the failures easier to lose. */
    const uint32_t notifications =
        CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING |
        CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_RX_FIFO0_OVERRUN |
        CAN_IT_RX_FIFO1_OVERRUN | CAN_IT_WAKEUP | CAN_IT_SLEEP_ACK |
        CAN_IT_ERROR_WARNING | CAN_IT_ERROR_PASSIVE | CAN_IT_BUSOFF |
        CAN_IT_LAST_ERROR_CODE | CAN_IT_ERROR;

    if (hcan == NULL) {
        return HAL_ERROR;
    }

    return HAL_CAN_DeactivateNotification(&hcan->hal, notifications);
}

/* HAL callbacks -------------------------------------------------------------*/

/**
 * @brief Resolve handle by instance and dispatch Tx-complete
 * @param hcan_ptr HAL handle from ISR
 * @param mailbox Mailbox index (0..2)
 */
static void CAN_NotifyTxFromIsr(CAN_HandleTypeDef *hcan_ptr, uint8_t mailbox)
{
    CAN_NotifyTxComplete(CAN_FromInstance(hcan_ptr->Instance), mailbox);
}

/**
 * @brief HAL Tx mailbox 0 complete callback
 * @param hcan_ptr HAL handle from ISR
 */
void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan_ptr)
{
    CAN_NotifyTxFromIsr(hcan_ptr, 0);
}

/**
 * @brief HAL Tx mailbox 1 complete callback
 * @param hcan_ptr HAL handle from ISR
 */
void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan_ptr)
{
    CAN_NotifyTxFromIsr(hcan_ptr, 1);
}

/**
 * @brief HAL Tx mailbox 2 complete callback
 * @param hcan_ptr HAL handle from ISR
 */
void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan_ptr)
{
    CAN_NotifyTxFromIsr(hcan_ptr, 2);
}

/**
 * @brief Extract a frame from a FIFO and dispatch the Rx callback
 * @param hcan_ptr HAL handle from ISR
 * @param fifo FIFO index (CAN_RX_FIFO0 or CAN_RX_FIFO1)
 */
static void CAN_HandleRxFifo(CAN_HandleTypeDef *hcan_ptr, uint32_t fifo)
{
    CAN_RxHeaderTypeDef rx_header;
    CAN_Frame frame = {0};
    CAN_Handle_t *hcan = CAN_FromInstance(hcan_ptr->Instance);

    if (hcan == NULL) {
        return;
    }

    if (HAL_CAN_GetRxMessage(&hcan->hal, fifo, &rx_header, frame.data) == HAL_OK) {
        frame.id = (rx_header.IDE == CAN_ID_STD) ? rx_header.StdId : rx_header.ExtId;
        frame.frame_type = (rx_header.IDE == CAN_ID_STD) ? CAN_FRAME_STANDARD : CAN_FRAME_EXTENDED;
        frame.data_length = rx_header.DLC;
        frame.remote_request = (rx_header.RTR == CAN_RTR_REMOTE);
        frame.timestamp = rx_header.Timestamp;

        CAN_IncrementRxCount(hcan);
        CAN_NotifyRx(hcan, &frame);
    }
}

/**
 * @brief HAL Rx FIFO 0 message pending callback
 * @param hcan_ptr HAL handle from ISR
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan_ptr)
{
    CAN_HandleRxFifo(hcan_ptr, CAN_RX_FIFO0);
}

/**
 * @brief HAL Rx FIFO 1 message pending callback
 * @param hcan_ptr HAL handle from ISR
 */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan_ptr)
{
    CAN_HandleRxFifo(hcan_ptr, CAN_RX_FIFO1);
}

/**
 * @brief HAL error callback
 * @param hcan_ptr HAL handle from ISR
 */
void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan_ptr)
{
    CAN_Handle_t *hcan = CAN_FromInstance(hcan_ptr->Instance);

    if (hcan != NULL) {
        CAN_NotifyError(hcan);
    }
}
