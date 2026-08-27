/**
 * @file uart.c
 * @brief Small helpers shared by all three UART modes (blocking/interrupt/DMA)
 *
 * Each mode has its own Init/Transmit/Receive functions elsewhere. This file
 * only holds the bits every mode needs: closing a link down, and keeping
 * track of which link is currently open.
 */

#include "uart.h"

#include "log.h"
#include <string.h>

/* Interrupts and HAL's setup code can't be passed our handle directly, so we
 * remember it here and they look it up when they need it. */
static UART_Handle_t *s_activeHandle = NULL;

void UART_SetActiveHandle(UART_Handle_t *handle)
{
    s_activeHandle = handle;
}

UART_Handle_t *UART_GetActiveHandle(void)
{
    return s_activeHandle;
}

UART_Status_t UART_DeInit(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        log_debug("UART handle or huart is NULL");
        return UART_ERROR;
    }

    /* Turn off every interrupt source so nothing fires after we tear down. */
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_TC);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_IDLE);

    /* Let HAL release the pins/clocks/DMA it set up for us. */
    if (HAL_UART_DeInit(handle->huart) != HAL_OK) {
        log_debug("UART deinitialization failed");
        return UART_ERROR;
    }

    /* Put the handle back to a clean, unused state. */
    handle->isInitialized = false;
    handle->rxBuffer = NULL;
    handle->rxSize = 0;
    handle->txComplete = false;
    handle->rxComplete = false;
    memset(&handle->config, 0, sizeof(UART_Config_t));

    if (s_activeHandle == handle) {
        s_activeHandle = NULL;
    }

    return UART_OK;
}

