/**
  ******************************************************************************
  * @file    crc_events.h
  * @brief   Callback registration and dispatch
  ******************************************************************************
  */

#ifndef CRC_EVENTS_H
#define CRC_EVENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "crc_types.h"

void CRC_RegisterCompleteCallback(CRC_CompleteCallback callback);
void CRC_RegisterErrorCallback(CRC_ErrorCallbackFn callback);

void CRC_NotifyComplete(uint32_t crc_value);
void CRC_NotifyError(CRC_ErrorType error);

#ifdef __cplusplus
}
#endif

#endif /* CRC_EVENTS_H */
