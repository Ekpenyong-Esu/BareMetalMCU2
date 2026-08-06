/**
 * @file uart_events.c
 * @brief HAL UART callbacks: turn interrupt events into buffered data and flags
 *
 * Everything here runs in interrupt context. The vectors themselves live in
 * Core/Src/stm32f4xx_it.c; HAL_UART_IRQHandler() dispatches into the callbacks
 * below. Nothing here knows which transfer mode is active — the handle's ops
 * table answers that.
 */

#include "uart_core.h"
#include "log.h"

/* Callbacks receive a HAL handle, not our handle. Resolve the owning link and
 * ignore events raised by any other UART in the system. */
static UART_Handle_t *OwnerOf(const UART_HandleTypeDef *huart)
{
    UART_Handle_t *handle = UART_GetActiveHandle();

    return (handle != NULL && handle->ops != NULL && handle->huart == huart) ? handle : NULL;
}

/* Reception is one-shot, so every completed transfer must re-arm it. */
static void RestartReceive(UART_Handle_t *handle)
{
    if (handle->ops->rearmReceive != NULL) {
        handle->ops->rearmReceive(handle);
    }
}

static void CompleteReception(UART_Handle_t *handle, uint16_t size)
{
    if (size > 0) {
        uint32_t stored = RingBuffer_PutBytes(&handle->rxRing, handle->rxBuffer, size);
        if (stored < size) {
            log_debug("UART RX ring full, dropped %u bytes", (unsigned)(size - stored));
        }
    }

    handle->rxComplete = true;
    RestartReceive(handle);
}

/**
 * @brief Idle line detected: a partial buffer has arrived
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL || Size == 0 || handle->ops->rearmReceive == NULL) {
        return;
    }

    CompleteReception(handle, Size);
}

/**
 * @brief Transmission finished
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL) {
        return;
    }

    handle->txComplete = true;
}

/**
 * @brief Requested number of bytes received
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL || handle->ops->completedReceiveSize == NULL) {
        return;
    }

    CompleteReception(handle, handle->ops->completedReceiveSize(handle, huart));
}

/**
 * @brief Line error: clear the cause, then get reception running again
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL) {
        return;
    }

    if (huart->ErrorCode & HAL_UART_ERROR_ORE) {
        log_debug("UART Overrun Error");
        __HAL_UART_CLEAR_OREFLAG(huart);
    }
    if (huart->ErrorCode & HAL_UART_ERROR_NE) {
        log_debug("UART Noise Error");
        __HAL_UART_CLEAR_NEFLAG(huart);
    }
    if (huart->ErrorCode & HAL_UART_ERROR_FE) {
        log_debug("UART Frame Error");
        __HAL_UART_CLEAR_FEFLAG(huart);
    }
    if (huart->ErrorCode & HAL_UART_ERROR_PE) {
        log_debug("UART Parity Error");
        __HAL_UART_CLEAR_PEFLAG(huart);
    }

    /* Abort before re-arming, otherwise the stuck error state survives. */
    HAL_UART_AbortReceive(huart);
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    if (handle->ops->recoverFromError != NULL) {
        handle->ops->recoverFromError(handle);
    }

    RestartReceive(handle);
}
