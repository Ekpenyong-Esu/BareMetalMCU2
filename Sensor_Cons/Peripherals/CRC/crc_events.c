/**
 ******************************************************************************
 * @file    crc_events.c
 * @brief   Callback registration and dispatch
 ******************************************************************************
 */

#include "crc_events.h"

static CRC_CompleteCallback s_completeCallback = NULL;
static CRC_ErrorCallbackFn s_errorCallback = NULL;

void CRC_RegisterCompleteCallback(CRC_CompleteCallback callback) {
    s_completeCallback = callback;
}
void CRC_RegisterErrorCallback(CRC_ErrorCallbackFn callback) {
    s_errorCallback = callback;
}

void CRC_NotifyComplete(uint32_t crc_value) {
    if (s_completeCallback != NULL) {
        s_completeCallback(crc_value);
    }
}

void CRC_NotifyError(CRC_ErrorType error) {
    if (s_errorCallback != NULL) {
        s_errorCallback(error);
    }
}
