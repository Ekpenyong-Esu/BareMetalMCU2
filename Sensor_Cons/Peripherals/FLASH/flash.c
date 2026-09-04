/**
 ******************************************************************************
 * @file    flash.c
 * @brief   Flash access control: lock state, completion and status reporting
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "flash_core.h"

/* Private defines -----------------------------------------------------------*/

/** @brief Every error flag the Flash controller can latch on this part */
#define FLASH_ALL_ERROR_FLAGS                                                                      \
    (FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |                   \
     FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR)

/* Public functions ----------------------------------------------------------*/

FLASH_StatusTypeDef FLASH_Unlock(void) {
    HAL_StatusTypeDef halStatus = HAL_FLASH_Unlock();

    return FLASH_ConvertHALStatus(halStatus);
}

FLASH_StatusTypeDef FLASH_Lock(void) {
    HAL_StatusTypeDef halStatus = HAL_FLASH_Lock();

    return FLASH_ConvertHALStatus(halStatus);
}

FLASH_StatusTypeDef FLASH_WaitForOperation(uint32_t timeout) {
    HAL_StatusTypeDef halStatus = FLASH_WaitForLastOperation(timeout);

    return FLASH_ConvertHALStatus(halStatus);
}

FLASH_StatusTypeDef FLASH_ConvertHALStatus(HAL_StatusTypeDef halStatus) {
    switch (halStatus) {
        case HAL_OK:
            return FLASH_STATUS_OK;
        case HAL_ERROR:
            return FLASH_STATUS_ERROR;
        case HAL_BUSY:
            return FLASH_STATUS_BUSY;
        case HAL_TIMEOUT:
            return FLASH_STATUS_ERROR_OP;
        default:
            return FLASH_STATUS_ERROR;
    }
}

void FLASH_ClearErrorFlags(void) {
    __HAL_FLASH_CLEAR_FLAG(FLASH_ALL_ERROR_FLAGS);
}

const char *FLASH_GetStatusString(FLASH_StatusTypeDef status) {
    switch (status) {
        case FLASH_STATUS_OK:
            return "FLASH_STATUS_OK";
        case FLASH_STATUS_ERROR:
            return "FLASH_STATUS_ERROR";
        case FLASH_STATUS_ERROR_PROGRAM:
            return "FLASH_STATUS_ERROR_PROGRAM";
        case FLASH_STATUS_ERROR_WRP:
            return "FLASH_STATUS_ERROR_WRP";
        case FLASH_STATUS_ERROR_OP:
            return "FLASH_STATUS_ERROR_OP";
        case FLASH_STATUS_INVALID_PARAM:
            return "FLASH_STATUS_INVALID_PARAM";
        case FLASH_STATUS_INVALID_ADDRESS:
            return "FLASH_STATUS_INVALID_ADDRESS";
        case FLASH_STATUS_BUSY:
            return "FLASH_STATUS_BUSY";
        default:
            return "UNKNOWN_STATUS";
    }
}
