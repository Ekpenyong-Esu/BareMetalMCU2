/**
  ******************************************************************************
  * @file    can_transfer.h
  * @brief   CAN frame transmit and receive
  ******************************************************************************
  */

#ifndef CAN_TRANSFER_H
#define CAN_TRANSFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

/* Queues a frame into a free transmit mailbox, waiting up to timeout for one to free up. */
HAL_StatusTypeDef CAN_Transmit(const CAN_Frame *frame, uint32_t timeout);
/* Pops one frame from the receive FIFO, waiting up to timeout for one to arrive. */
HAL_StatusTypeDef CAN_Receive(CAN_Frame *frame, uint32_t timeout);
/* Whether at least one transmit mailbox is free right now. */
bool CAN_IsTransmitMailboxAvailable(void);
/* Whether the given receive FIFO has at least one pending frame. */
bool CAN_IsReceivePending(uint8_t fifo_number);
/* Number of frames currently pending in the given receive FIFO. */
uint8_t CAN_GetReceivePendingCount(uint8_t fifo_number);
/* Cancels a queued but not-yet-sent transmission. */
HAL_StatusTypeDef CAN_AbortTransmit(uint8_t mailbox_number);

#ifdef __cplusplus
}
#endif

#endif /* CAN_TRANSFER_H */
