/**
 * @file uart_blocking.c
 * @brief Blocking transfer mode (polling)
 *
 * The CPU waits inside each transfer call until the transfer completes or
 * times out. Nothing runs in interrupt context, so this mode is a plain,
 * independently-callable API: no vtable, no dispatch, no callbacks.
 *
 * Characteristics:
 * - Simplest to use and debug
 * - CPU is blocked during transfer (not suitable for high throughput)
 * - No interrupts enabled (RXNE, TC, IDLE, ERR all disabled)
 * - Uses HAL_UART_Transmit() and HAL_UARTEx_ReceiveToIdle()
 * - ReceiveToIdle returns on line idle, so variable-length frames work naturally
 *
 * Initialization:
 * - UART_Blocking_Init() configures UART, disables all interrupts
 * - Publishes handle as active before HAL_UART_Init() for MSP
 *
 * Transmit:
 * - HAL_UART_Transmit() polls TXE/TC flags until all bytes shifted out
 *
 * Receive:
 * - HAL_UARTEx_ReceiveToIdle() returns when requested bytes received OR
 *   line goes idle (no data for ~1 character time)
 * - On timeout with some data: returns UART_OK (partial frame)
 * - On timeout with no data: returns UART_TIMEOUT_ERROR
 * - On overrun/framing/noise error: clears ORE flag, returns UART_ERROR
 */

#include "uart_blocking.h"
#include "uart.h"
#include "uart_config.h"
#include "log.h"
#include <string.h>

/**
 * @brief Open a UART link in blocking mode
 *
 * Configures the UART peripheral with the given settings, disables all
 * interrupt sources (RXNE, TC, PE, ERR, IDLE), and marks the handle as
 * initialized. If the handle was already open, it is deinitialized first.
 *
 * @param handle UART handle to populate (must have huart, rxBuffer, rxSize set)
 * @param config Desired configuration (baud rate, word length, stop bits, parity)
 * @retval UART_OK on success, UART_ERROR on invalid args or HAL init failure
 */
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

    /* The CPU polls the peripheral directly, so every interrupt source stays off. */
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_TC);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_IDLE);

    handle->isInitialized = true;
    log_debug("UART: opened in blocking mode");
    return UART_OK;
}

/* Both transfer directions share the same preconditions. */
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

/**
 * @brief Send data, waiting for it to leave the peripheral
 *
 * Calls HAL_UART_Transmit() which polls the TXE/TC flags until all bytes
 * are shifted out or timeout expires. The CPU does not return until done.
 *
 * @param handle  UART handle (must be initialized in blocking mode)
 * @param data    Bytes to send
 * @param size    Number of bytes
 * @param timeout Milliseconds to wait for transfer to complete
 * @retval UART_OK on success, UART_ERROR on invalid args, UART_TIMEOUT_ERROR on timeout
 */
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

/**
 * @brief Receive data, waiting for it to arrive
 *
 * Uses HAL_UARTEx_ReceiveToIdle() which returns when either:
 * - The requested number of bytes is received, OR
 * - The line goes idle (no data for ~1 character time)
 *
 * This means a frame shorter than @p size is a normal success. The actual
 * byte count is written to @p received. Bytes past @p received are left
 * untouched; the caller must not assume a terminator.
 *
 * On timeout with some data received, returns UART_OK (partial frame).
 * On timeout with no data, returns UART_TIMEOUT_ERROR.
 * On overrun/framing/noise error, clears ORE flag and returns UART_ERROR.
 *
 * @param handle   UART handle (must be initialized in blocking mode)
 * @param data     Destination buffer
 * @param size     Capacity of @p data in bytes
 * @param received Out: bytes actually written (set even on failure)
 * @param timeout  Milliseconds to wait
 * @retval UART_OK on success (including partial frame), UART_TIMEOUT_ERROR if nothing arrived,
 *         UART_ERROR on overrun/framing/noise error or invalid args
 */
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

    /* ReceiveToIdle returns on a gap in the stream, so a short frame does not
       have to wait out the whole timeout. */
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle(handle->huart, data, size, received, timeout);

    if (status == HAL_TIMEOUT) {
        /* Data that arrived before the deadline still counts as a frame. */
        return (*received > 0) ? UART_OK : UART_TIMEOUT_ERROR;
    }

    if (status != HAL_OK) {
        /* Bytes that land while the CPU is elsewhere latch ORE; clear it here
           or every later call fails too. Also covers PE/FE/NE. */
        __HAL_UART_CLEAR_OREFLAG(handle->huart);
        handle->huart->ErrorCode = HAL_UART_ERROR_NONE;
        log_debug("Blocking UART Receive failed: %d", status);
        return UART_ERROR;
    }

    return UART_OK;
}
