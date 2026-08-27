/**
 * @file uart_blocking.c
 * @brief Blocking mode: the simplest way to use UART
 *
 * "Blocking" means the CPU just sits and waits until the send/receive is
 * done (or it times out). No interrupts, no background copying - easy to
 * follow, but the CPU can't do anything else while it waits.
 */

#include "uart_blocking.h"
#include "uart.h"
#include "uart_config.h"
#include "log.h"
#include <string.h>

UART_Status_t UART_Blocking_Init(UART_Handle_t* handle, const UART_Config_t* config)
{
    if (handle == NULL || config == NULL || config->instance == NULL || handle->huart == NULL) {
        log_debug("Blocking UART: handle, huart or config is NULL");
        return UART_ERROR;
    }

    if (handle->isInitialized) {
        UART_DeInit(handle);
    }

    handle->config = *config;
    memset(handle->huart, 0, sizeof(UART_HandleTypeDef));

    /* Publish before HAL_UART_Init(), which calls into HAL_UART_MspInit(). */
    UART_SetActiveHandle(handle);

    handle->huart->Instance = config->instance;
    handle->huart->Init.BaudRate = config->baudRate;
    handle->huart->Init.WordLength = config->wordLength;
    handle->huart->Init.StopBits = config->stopBits;
    handle->huart->Init.Parity = config->parity;
    handle->huart->Init.Mode = UART_DEFAULT_MODE;
    handle->huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    handle->huart->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(handle->huart) != HAL_OK) {
        log_debug("Blocking UART initialization failed");
        return UART_ERROR;
    }

    /* Blocking mode never uses interrupts, so make sure they're all off. */
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_TC);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_IDLE);

    handle->isInitialized = true;
    log_debug("UART: opened in blocking mode");
    return UART_OK;
}

/* Sanity checks shared by both send and receive, so we don't repeat them. */
static bool IsReadyForTransfer(const UART_Handle_t *handle, const void *data, uint16_t size)
{
    if (handle == NULL) {
        log_debug("UART handle is NULL");
        return false;
    }

    if (data == NULL) {
        log_debug("UART data is NULL");
        return false;
    }

    if (size == 0) {
        log_debug("UART size is 0");
        return false;
    }

    if (!handle->isInitialized) {
        log_debug("UART not initialized");
        return false;
    }

    return true;
}

UART_Status_t UART_Blocking_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (!IsReadyForTransfer(handle, data, size)) {
        return UART_ERROR;
    }

    if (HAL_UART_Transmit(handle->huart, (uint8_t*)data, size, timeout) != HAL_OK) {
        log_debug("Blocking UART Transmit failed");
        return UART_ERROR;
    }

    return UART_OK;
}

UART_Status_t UART_Blocking_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size,
                                    uint16_t* received, uint32_t timeout)
{
    if (received == NULL) {
        log_debug("UART received-count pointer is NULL");
        return UART_ERROR;
    }

    *received = 0;

    if (!IsReadyForTransfer(handle, data, size)) {
        return UART_ERROR;
    }

    /* "ToIdle" means it also stops early if the sender pauses, so a short
       message doesn't force us to wait for the full timeout. */
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle(handle->huart, data, size, received, timeout);

    if (status == HAL_TIMEOUT) {
        /* Timed out, but if some bytes did arrive, treat that as a success. */
        return (*received > 0) ? UART_OK : UART_TIMEOUT_ERROR;
    }

    if (status != HAL_OK) {
        /* Clear any leftover error flag, otherwise every future call fails too. */
        __HAL_UART_CLEAR_OREFLAG(handle->huart);
        handle->huart->ErrorCode = HAL_UART_ERROR_NONE;
        log_debug("Blocking UART Receive failed: %d", status);
        return UART_ERROR;
    }

    return UART_OK;
}
