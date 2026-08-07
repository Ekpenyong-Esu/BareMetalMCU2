/**
  ******************************************************************************
  * @file    can_transfer.c
  * @brief   CAN frame transmit and receive
  ******************************************************************************
  */

#include "can_transfer.h"
#include "can_core.h"
#include "can_events.h"

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

HAL_StatusTypeDef CAN_Transmit(const CAN_Frame *frame, uint32_t timeout)
{
    CAN_TxHeaderTypeDef tx_header = {0};
    uint32_t tx_mailbox = 0;
    CAN_HandleTypeDef *hcan = CAN_GetHandle();

    if (frame == NULL || frame->data_length > CAN_MAX_DATA_LENGTH) {
        return HAL_ERROR;
    }

    tx_header.StdId = (frame->frame_type == CAN_FRAME_STANDARD) ? frame->id : 0;
    tx_header.ExtId = (frame->frame_type == CAN_FRAME_EXTENDED) ? frame->id : 0;
    tx_header.IDE = (frame->frame_type == CAN_FRAME_STANDARD) ? CAN_ID_STD : CAN_ID_EXT;
    tx_header.RTR = frame->remote_request ? CAN_RTR_REMOTE : CAN_RTR_DATA;
    tx_header.DLC = frame->data_length;
    tx_header.TransmitGlobalTime = DISABLE;

    if (HAL_CAN_AddTxMessage(hcan, &tx_header, (uint8_t *)(uintptr_t)frame->data,
                             &tx_mailbox) != HAL_OK) {
        return HAL_ERROR;
    }

    if (timeout > 0U) {
        uint32_t start = HAL_GetTick();
        while (HAL_CAN_IsTxMessagePending(hcan, tx_mailbox)) {
            if ((HAL_GetTick() - start) > timeout) {
                /* Drop the request instead of leaving the mailbox occupied by
                   a frame the caller has already given up on. */
                (void)HAL_CAN_AbortTxRequest(hcan, tx_mailbox);
                return HAL_TIMEOUT;
            }
        }
    }

    CAN_IncrementTxCount();

    return HAL_OK;
}

HAL_StatusTypeDef CAN_Receive(CAN_Frame *frame, uint32_t timeout)
{
    CAN_RxHeaderTypeDef rx_header;
    CAN_HandleTypeDef *hcan = CAN_GetHandle();

    if (frame == NULL) {
        return HAL_ERROR;
    }

    if (!CAN_IsReceivePending(0) && !CAN_IsReceivePending(1)) {
        if (timeout == 0U) {
            return HAL_ERROR;
        }
        uint32_t start = HAL_GetTick();
        while (!CAN_IsReceivePending(0) && !CAN_IsReceivePending(1)) {
            if ((HAL_GetTick() - start) > timeout) {
                return HAL_TIMEOUT;
            }
        }
    }

    uint32_t fifo = CAN_IsReceivePending(0) ? CAN_RX_FIFO0 : CAN_RX_FIFO1;

    if (HAL_CAN_GetRxMessage(hcan, fifo, &rx_header, frame->data) != HAL_OK) {
        return HAL_ERROR;
    }

    CAN_FillFrameFromRxHeader(frame, &rx_header);
    CAN_IncrementRxCount();
    CAN_NotifyRx(frame);

    return HAL_OK;
}

bool CAN_IsTransmitMailboxAvailable(void)
{
    return (HAL_CAN_GetTxMailboxesFreeLevel(CAN_GetHandle()) > 0);
}

bool CAN_IsReceivePending(uint8_t fifo_number)
{
    uint32_t fifo;

    if (fifo_number > 1U) {
        return false;
    }

    fifo = (fifo_number == 0U) ? CAN_RX_FIFO0 : CAN_RX_FIFO1;
    return (HAL_CAN_GetRxFifoFillLevel(CAN_GetHandle(), fifo) > 0);
}

uint8_t CAN_GetReceivePendingCount(uint8_t fifo_number)
{
    uint32_t fifo;

    if (fifo_number > 1U) {
        return 0;
    }

    fifo = (fifo_number == 0U) ? CAN_RX_FIFO0 : CAN_RX_FIFO1;
    return HAL_CAN_GetRxFifoFillLevel(CAN_GetHandle(), fifo);
}

HAL_StatusTypeDef CAN_AbortTransmit(uint8_t mailbox_number)
{
    uint32_t mailbox;

    if (mailbox_number >= CAN_MAX_MAILBOXES) {
        return HAL_ERROR;
    }

    mailbox = (mailbox_number == 0U) ? CAN_TX_MAILBOX0 :
              (mailbox_number == 1U) ? CAN_TX_MAILBOX1 : CAN_TX_MAILBOX2;

    return HAL_CAN_AbortTxRequest(CAN_GetHandle(), mailbox);
}
