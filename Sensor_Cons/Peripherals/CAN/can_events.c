/**
  ******************************************************************************
  * @file    can_events.c
  * @brief   HAL callbacks and interrupt enable/disable
  ******************************************************************************
  */

#include "can_events.h"
#include "can_core.h"

static CAN_TxCallback s_txCallback = NULL;
static CAN_RxCallback s_rxCallback = NULL;
static CAN_ErrorCallbackFn s_errorCallback = NULL;

void CAN_RegisterTxCallback(CAN_TxCallback callback) { s_txCallback = callback; }
void CAN_RegisterRxCallback(CAN_RxCallback callback) { s_rxCallback = callback; }
void CAN_RegisterErrorCallback(CAN_ErrorCallbackFn callback) { s_errorCallback = callback; }

void CAN_NotifyRx(const CAN_Frame *frame)
{
    if (s_rxCallback != NULL) {
        s_rxCallback(frame);
    }
}

void CAN_NotifyTxComplete(uint8_t mailbox)
{
    if (s_txCallback != NULL) {
        s_txCallback(mailbox);
    }
}

void CAN_NotifyError(void)
{
    CAN_ErrorType error = CAN_GetErrorType();

    if (error != CAN_ERROR_NONE) {
        CAN_RecordError(error);
        if (s_errorCallback != NULL) {
            s_errorCallback(error);
        }
    }
}

HAL_StatusTypeDef CAN_EnableInterrupts(void)
{
    CAN_HandleTypeDef *hcan = CAN_GetHandle();

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

    for (size_t i = 0; i < (sizeof(notifications) / sizeof(notifications[0])); i++) {
        HAL_StatusTypeDef status = HAL_CAN_ActivateNotification(hcan, notifications[i]);

        if (status != HAL_OK) {
            return status;
        }
    }

    return HAL_OK;
}

HAL_StatusTypeDef CAN_DisableInterrupts(void)
{
    CAN_HandleTypeDef *hcan = CAN_GetHandle();

    /* The HAL takes a mask, so one call reports one result; deactivating them
       one at a time only made the failures easier to lose. */
    const uint32_t notifications =
        CAN_IT_TX_MAILBOX_EMPTY | CAN_IT_RX_FIFO0_MSG_PENDING |
        CAN_IT_RX_FIFO1_MSG_PENDING | CAN_IT_RX_FIFO0_OVERRUN |
        CAN_IT_RX_FIFO1_OVERRUN | CAN_IT_WAKEUP | CAN_IT_SLEEP_ACK |
        CAN_IT_ERROR_WARNING | CAN_IT_ERROR_PASSIVE | CAN_IT_BUSOFF |
        CAN_IT_LAST_ERROR_CODE | CAN_IT_ERROR;

    return HAL_CAN_DeactivateNotification(hcan, notifications);
}

/* ---- HAL callbacks ---- */

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef *hcan_ptr)
{
    (void)hcan_ptr;
    CAN_NotifyTxComplete(0);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef *hcan_ptr)
{
    (void)hcan_ptr;
    CAN_NotifyTxComplete(1);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef *hcan_ptr)
{
    (void)hcan_ptr;
    CAN_NotifyTxComplete(2);
}

/* FIFO0 and FIFO1 share the same extraction logic */
static void CAN_HandleRxFifo(CAN_HandleTypeDef *hcan_ptr, uint32_t fifo)
{
    CAN_RxHeaderTypeDef rx_header;
    CAN_Frame frame = {0};

    (void)hcan_ptr;

    if (HAL_CAN_GetRxMessage(CAN_GetHandle(), fifo, &rx_header, frame.data) == HAL_OK) {
        frame.id = (rx_header.IDE == CAN_ID_STD) ? rx_header.StdId : rx_header.ExtId;
        frame.frame_type = (rx_header.IDE == CAN_ID_STD) ? CAN_FRAME_STANDARD : CAN_FRAME_EXTENDED;
        frame.data_length = rx_header.DLC;
        frame.remote_request = (rx_header.RTR == CAN_RTR_REMOTE);
        frame.timestamp = rx_header.Timestamp;

        CAN_IncrementRxCount();
        CAN_NotifyRx(&frame);
    }
}

void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan_ptr)
{
    CAN_HandleRxFifo(hcan_ptr, CAN_RX_FIFO0);
}

void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef *hcan_ptr)
{
    CAN_HandleRxFifo(hcan_ptr, CAN_RX_FIFO1);
}

void HAL_CAN_ErrorCallback(CAN_HandleTypeDef *hcan_ptr)
{
    (void)hcan_ptr;
    CAN_NotifyError();
}
