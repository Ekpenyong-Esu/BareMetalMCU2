/**
 * @file uart_blocking.c
 * @brief Blocking transfer mode
 */

#include "uart_blocking.h"
#include "log.h"
#include <string.h>

static UART_Status_t Init(UART_Handle_t* handle)
{
    /* Blocking transfers poll the peripheral, so every interrupt source stays off. */
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_TC);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_IDLE);

    return UART_OK;
}

static UART_Status_t Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (HAL_UART_Transmit(handle->huart, (uint8_t*)data, size, timeout) != HAL_OK) {
        log_debug("Blocking UART Transmit failed");
        return UART_ERROR;
    }

    return UART_OK;
}

static UART_Status_t Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    uint16_t received = 0;

    /* ReceiveToIdle returns on a gap in the stream, so a short frame does not
       have to wait out the whole timeout. */
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle(handle->huart, data, size, &received, timeout);

    if (status == HAL_TIMEOUT && received == 0) {
        log_debug("Blocking UART Receive timeout");
        return UART_TIMEOUT_ERROR;
    }

    if (status != HAL_OK && status != HAL_TIMEOUT) {
        log_debug("Blocking UART Receive failed: %d", status);
        return UART_ERROR;
    }

    /* A short frame is still a success; blank the tail so the caller never
       reads bytes left over from an earlier transfer. */
    if (received < size) {
        memset(data + received, 0, size - received);
    }

    return UART_OK;
}

const UART_ModeOps_t UART_BlockingOps = {
    .name = "blocking",
    .init = Init,
    .transmit = Transmit,
    .receive = Receive,
    .rearmReceive = NULL,
    .completedReceiveSize = NULL,
    .recoverFromError = NULL,
};
