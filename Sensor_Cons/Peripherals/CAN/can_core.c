/**
  ******************************************************************************
  * @file    can_core.c
  * @brief   Lifecycle and status for the CAN peripheral
  ******************************************************************************
  */

#include "can_core.h"
#include "can_baudrate.h"
#include "can_events.h"
#include <string.h>
#include "log.h"

static CAN_HandleTypeDef s_hcan;
static CAN_Status s_status = {0};

/* ---- Error mapping table ---- */

/* Ordered most severe first: HAL reports several flags at once and the first
   match wins, so a stuff error must not hide a bus-off condition. */
static const struct {
    uint32_t hal_flag;
    CAN_ErrorType error;
} s_errorMap[] = {
    { HAL_CAN_ERROR_BOF, CAN_ERROR_BUS_OFF       },
    { HAL_CAN_ERROR_EPV, CAN_ERROR_BUS_PASSIVE   },
    { HAL_CAN_ERROR_EWG, CAN_ERROR_BUS_WARNING   },
    { HAL_CAN_ERROR_ACK, CAN_ERROR_ACK           },
    { HAL_CAN_ERROR_STF, CAN_ERROR_STUFF         },
    { HAL_CAN_ERROR_FOR, CAN_ERROR_FORM          },
    { HAL_CAN_ERROR_BR,  CAN_ERROR_BIT_RECESSIVE },
    { HAL_CAN_ERROR_BD,  CAN_ERROR_BIT_DOMINANT  },
    { HAL_CAN_ERROR_CRC, CAN_ERROR_CRC           },
};

#define CAN_ERROR_MAP_SIZE (sizeof(s_errorMap) / sizeof(s_errorMap[0]))

CAN_ErrorType CAN_GetErrorType(void)
{
    uint32_t error = HAL_CAN_GetError(&s_hcan);

    for (size_t i = 0; i < CAN_ERROR_MAP_SIZE; i++) {
        if (error & s_errorMap[i].hal_flag) {
            return s_errorMap[i].error;
        }
    }

    return CAN_ERROR_NONE;
}

void CAN_RecordError(CAN_ErrorType error)
{
    s_status.error_count++;
    s_status.last_error = error;
}

void CAN_IncrementTxCount(void) { s_status.tx_count++; }
void CAN_IncrementRxCount(void) { s_status.rx_count++; }

CAN_HandleTypeDef *CAN_GetHandle(void) { return &s_hcan; }

HAL_StatusTypeDef CAN_Init(const CAN_Config *config)
{
    HAL_StatusTypeDef status;

    if (config == NULL) {
        return HAL_ERROR;
    }

    log_debug("CAN: Initializing CAN");

    memset(&s_status, 0, sizeof(s_status));

    s_hcan.Instance = CAN1;
    s_hcan.Init.Mode = config->mode;
    s_hcan.Init.AutoRetransmission = config->auto_retransmission ? ENABLE : DISABLE;
    s_hcan.Init.AutoBusOff = config->auto_bus_off_recovery ? ENABLE : DISABLE;
    s_hcan.Init.TimeTriggeredMode = config->time_triggered_comm ? ENABLE : DISABLE;

    status = CAN_ApplyBaudRate(&s_hcan, config);
    if (status != HAL_OK) {
        return status;
    }

    status = HAL_CAN_Init(&s_hcan);
    if (status != HAL_OK) {
        return status;
    }

    status = HAL_CAN_Start(&s_hcan);
    if (status != HAL_OK) {
        return status;
    }

    s_status.initialized = true;

    log_debug("CAN: CAN initialized successfully");

    return HAL_OK;
}

HAL_StatusTypeDef CAN_DeInit(void)
{
    HAL_StatusTypeDef status;

    if (!s_status.initialized) {
        return HAL_ERROR;
    }

    status = HAL_CAN_Stop(&s_hcan);
    if (status != HAL_OK) {
        return status;
    }

    status = HAL_CAN_DeInit(&s_hcan);
    if (status != HAL_OK) {
        return status;
    }

    memset(&s_status, 0, sizeof(s_status));

    /* Reset callbacks */
    CAN_RegisterTxCallback(NULL);
    CAN_RegisterRxCallback(NULL);
    CAN_RegisterErrorCallback(NULL);

    return HAL_OK;
}

HAL_StatusTypeDef CAN_SetMode(CAN_OperatingMode mode)
{
    HAL_StatusTypeDef status;
    uint32_t previous_mode = s_hcan.Init.Mode;

    if (!s_status.initialized) {
        return HAL_ERROR;
    }

    status = HAL_CAN_Stop(&s_hcan);
    if (status != HAL_OK) {
        return status;
    }

    s_hcan.Init.Mode = mode;

    status = HAL_CAN_Init(&s_hcan);
    if (status != HAL_OK) {
        s_hcan.Init.Mode = previous_mode;
        s_status.initialized = false;
        return status;
    }

    status = HAL_CAN_Start(&s_hcan);
    if (status != HAL_OK) {
        s_status.initialized = false;
    }

    return status;
}

HAL_StatusTypeDef CAN_GetStatus(CAN_Status *status)
{
    CAN_ErrorType live;

    if (status == NULL) {
        return HAL_ERROR;
    }

    memcpy(status, &s_status, sizeof(CAN_Status));

    status->tx_mailbox_free = HAL_CAN_GetTxMailboxesFreeLevel(&s_hcan);
    status->rx_messages_pending = HAL_CAN_GetRxFifoFillLevel(&s_hcan, CAN_RX_FIFO0) +
                                  HAL_CAN_GetRxFifoFillLevel(&s_hcan, CAN_RX_FIFO1);

    /* Only override the recorded error while the condition is still live;
       otherwise the last fault would vanish as soon as HAL cleared it. */
    live = CAN_GetErrorType();
    if (live != CAN_ERROR_NONE) {
        status->last_error = live;
    }

    return HAL_OK;
}

HAL_StatusTypeDef CAN_ClearErrors(void)
{
    /* Only the FIFO overrun flags are writable. EWG/EPV/BOF are read-only
       status bits in ESR that the controller owns; attempting to clear them
       expands to a no-op expression. */
    __HAL_CAN_CLEAR_FLAG(&s_hcan, CAN_FLAG_FOV0);
    __HAL_CAN_CLEAR_FLAG(&s_hcan, CAN_FLAG_FOV1);

    s_hcan.ErrorCode = HAL_CAN_ERROR_NONE;

    s_status.error_count = 0;
    s_status.last_error = CAN_ERROR_NONE;

    return HAL_OK;
}
