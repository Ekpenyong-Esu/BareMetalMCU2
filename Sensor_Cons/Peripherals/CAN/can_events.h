/**
  ******************************************************************************
  * @file    can_events.h
  * @brief   HAL callbacks and interrupt enable/disable
  ******************************************************************************
  */

#ifndef CAN_EVENTS_H
#define CAN_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

/* Sets the callback invoked from CAN_NotifyTxComplete(). */
void CAN_RegisterTxCallback(CAN_TxCallback callback);
/* Sets the callback invoked from CAN_NotifyRx(). */
void CAN_RegisterRxCallback(CAN_RxCallback callback);
/* Sets the callback invoked from CAN_NotifyError(). */
void CAN_RegisterErrorCallback(CAN_ErrorCallbackFn callback);

/* Enables the TX/RX/error interrupt sources used by this driver. */
HAL_StatusTypeDef CAN_EnableInterrupts(void);
/* Disables the TX/RX/error interrupt sources used by this driver. */
HAL_StatusTypeDef CAN_DisableInterrupts(void);

/* Called by can_core / can_transfer to dispatch registered callbacks */
void CAN_NotifyRx(const CAN_Frame *frame);
void CAN_NotifyTxComplete(uint8_t mailbox);
void CAN_NotifyError(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_EVENTS_H */
