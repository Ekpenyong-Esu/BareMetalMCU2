/**
  ******************************************************************************
  * @file    can_core.h
  * @brief   Lifecycle and status for the CAN peripheral
  ******************************************************************************
  */

#ifndef CAN_CORE_H
#define CAN_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "can_types.h"

/* Brings up the CAN peripheral with the given bit timing/filter settings. */
HAL_StatusTypeDef CAN_Init(const CAN_Config *config);
/* Tears down the peripheral so it can be re-initialised. */
HAL_StatusTypeDef CAN_DeInit(void);
/* Switches between normal/loopback/silent operating modes. */
HAL_StatusTypeDef CAN_SetMode(CAN_OperatingMode mode);
/* Reports bus state, error counters and last error type. */
HAL_StatusTypeDef CAN_GetStatus(CAN_Status *status);
/* Clears the recorded error state and counters. */
HAL_StatusTypeDef CAN_ClearErrors(void);
/* Access to the underlying HAL handle. */
CAN_HandleTypeDef *CAN_GetHandle(void);

/* Error mapping — used by can_events */
/* Last error type recorded by CAN_RecordError(). */
CAN_ErrorType CAN_GetErrorType(void);
/* Records the most recent error type, for CAN_GetErrorType()/CAN_GetStatus(). */
void CAN_RecordError(CAN_ErrorType error);

/* Statistics — used by can_transfer / can_events */
/* Bumps the transmitted-frame counter. */
void CAN_IncrementTxCount(void);
/* Bumps the received-frame counter. */
void CAN_IncrementRxCount(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_CORE_H */
