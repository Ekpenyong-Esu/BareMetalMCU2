/**
 * @file uart.c
 * @brief Shared UART utilities: teardown, active-link lookup, flag waits
 *
 * Each transfer mode owns its own Init/Transmit/Receive; this file never
 * includes a mode header and has no dispatch table, matching how tim_clock.c
 * knows nothing about tim_pwm.c or tim_ic.c.
 */

#include "uart.h"

#include "log.h"
#include <string.h>

/* The MSP and the interrupt vectors run without a caller-supplied handle, so
 * the driver publishes the link it is currently serving. */
static UART_Handle_t *s_activeHandle = NULL;

void UART_SetActiveHandle(UART_Handle_t *handle)
{
    s_activeHandle = handle;
}

UART_Handle_t *UART_GetActiveHandle(void)
{
    return s_activeHandle;
}

UART_Status_t UART_WaitForFlag(volatile bool *flag, uint32_t timeout)
{
    uint32_t startTick = HAL_GetTick();

    while (!*flag) {
        if ((HAL_GetTick() - startTick) > timeout) {
            return UART_TIMEOUT_ERROR;
        }
    }

    return UART_OK;
}

UART_Status_t UART_DeInit(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        log_debug("UART handle or huart is NULL");
        return UART_ERROR;
    }

    /* Disable all UART interrupts */
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_TC);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_IDLE);

    /* HAL_UART_DeInit() releases the MSP resources on our behalf. */
    if (HAL_UART_DeInit(handle->huart) != HAL_OK) {
        log_debug("UART deinitialization failed");
        return UART_ERROR;
    }

    /* Reset handle state */
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

