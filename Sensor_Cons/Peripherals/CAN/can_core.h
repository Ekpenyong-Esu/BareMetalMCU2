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

HAL_StatusTypeDef CAN_Init(const CAN_Config *config);
HAL_StatusTypeDef CAN_DeInit(void);
HAL_StatusTypeDef CAN_SetMode(CAN_OperatingMode mode);
HAL_StatusTypeDef CAN_GetStatus(CAN_Status *status);
HAL_StatusTypeDef CAN_ClearErrors(void);
CAN_HandleTypeDef *CAN_GetHandle(void);

/* Error mapping — used by can_events */
CAN_ErrorType CAN_GetErrorType(void);
void CAN_RecordError(CAN_ErrorType error);

/* Statistics — used by can_transfer / can_events */
void CAN_IncrementTxCount(void);
void CAN_IncrementRxCount(void);

#ifdef __cplusplus
}
#endif

#endif /* CAN_CORE_H */
