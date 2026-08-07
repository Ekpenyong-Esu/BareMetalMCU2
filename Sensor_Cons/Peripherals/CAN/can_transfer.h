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

HAL_StatusTypeDef CAN_Transmit(const CAN_Frame *frame, uint32_t timeout);
HAL_StatusTypeDef CAN_Receive(CAN_Frame *frame, uint32_t timeout);
bool CAN_IsTransmitMailboxAvailable(void);
bool CAN_IsReceivePending(uint8_t fifo_number);
uint8_t CAN_GetReceivePendingCount(uint8_t fifo_number);
HAL_StatusTypeDef CAN_AbortTransmit(uint8_t mailbox_number);

#ifdef __cplusplus
}
#endif

#endif /* CAN_TRANSFER_H */
