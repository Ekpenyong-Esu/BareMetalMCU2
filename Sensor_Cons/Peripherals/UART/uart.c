/**
 * @file uart.c
 * @brief Shared UART utilities: teardown, active-link lookup, flag waits
 *
 * Each transfer mode owns its own Init/Transmit/Receive; this file never
 * includes a mode header and has no dispatch table, matching how tim_clock.c
 * knows nothing about tim_pwm.c or tim_ic.c.
 *
 * Active Handle Registry:
 * - MSP (HAL_UART_MspInit) and ISR callbacks run without a caller-supplied handle
 * - Each mode's Init() calls UART_SetActiveHandle() before HAL_UART_Init()
 * - MSP/ISR use UART_GetActiveHandle() to find the correct link
 * - Only one handle can be "active" at a time (the most recently initialized)
 *
 * Flag Waiting:
 * - UART_WaitForFlag() busy-waits on a volatile boolean
 * - Used by interrupt/DMA modes to wait for txComplete/rxComplete flags
 * - Timeout uses HAL_GetTick() for millisecond resolution
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

/**
 * @brief Spin until a completion flag is raised
 *
 * Busy-waits on a volatile boolean flag (typically txComplete or rxComplete
 * set by interrupt callbacks). Uses HAL_GetTick() for timeout.
 *
 * @param flag    Pointer to volatile flag that an interrupt callback will set
 * @param timeout Milliseconds to wait; 0 returns UART_TIMEOUT_ERROR unless
 *                the flag is already raised
 * @retval UART_OK once raised, UART_TIMEOUT_ERROR if the wait expired
 */
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

/**
 * @brief Close a UART link and release its hardware resources
 *
 * Disables all UART interrupts (RXNE, TC, PE, ERR, IDLE), calls
 * HAL_UART_DeInit() to release MSP resources (GPIO, clocks, DMA),
 * and clears the handle state. If this handle was the active one,
 * the active handle registry is cleared.
 *
 * @param handle UART handle to deinitialize
 * @retval UART_OK on success, UART_ERROR if handle/huart is NULL or HAL fails
 */
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

