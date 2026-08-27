/**
 * @file uart.c
 * @brief Small helpers shared by the UART driver
 *
 * The transfer mode lives in uart_blocking.c. This file only holds the bits
 * that are mode-agnostic: closing a link down, and keeping track of which
 * link is currently open.
 */

#include "uart.h"

#include "log.h"
#include <string.h>

/* HAL's setup code can't be passed our handle directly, so we remember it
 * here and HAL_UART_MspInit() looks it up when it needs it. */
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

    /* Let HAL release the pins and clocks it set up for us. */
    if (HAL_UART_DeInit(handle->huart) != HAL_OK) {
        log_debug("UART deinitialization failed");
        return UART_ERROR;
    }

    /* Put the handle back to a clean, unused state. */
    handle->isInitialized = false;
    memset(&handle->config, 0, sizeof(UART_Config_t));

    if (s_activeHandle == handle) {
        s_activeHandle = NULL;
    }

    return UART_OK;
}

