/**
 * @file uart_events.c
 * @brief HAL UART callbacks: turn interrupt events into buffered data and flags
 *
 * Everything here runs in interrupt context. The vectors themselves live in
 * Core/Src/stm32f4xx_it.c; HAL_UART_IRQHandler() dispatches into the callbacks
 * below. This is the one place that has to know which mode is active, since
 * HAL only exposes one callback per event regardless of mode.
 */

#include "uart.h"
#include "uart_interrupt.h"
#include "uart_dma.h"
#include "log.h"

/* Callbacks receive a HAL handle, not our handle. Resolve the owning link by
 * instance and ignore events raised by a UART this driver does not own. */
static UART_Handle_t *OwnerOf(const UART_HandleTypeDef *huart) {
    UART_Handle_t *handle = UART_FromInstance(huart->Instance);

    return (handle != NULL && handle->isInitialized && handle->huart == huart) ? handle : NULL;
}

/* Reception is one-shot, so every completed transfer must re-arm it; blocking
 * mode never receives asynchronously, so it has nothing to re-arm. */
static void RestartReceive(UART_Handle_t *handle) {
    switch (handle->config.mode) {
        case UART_MODE_INTERRUPT:
            if (!UART_Interrupt_Rearm(handle)) {
                log_error("UART: could not re-arm reception");
            }
            break;
        case UART_MODE_DMA:
            UART_DMA_Rearm(handle);
            break;
        default:
            break;
    }
}

static void CompleteReception(UART_Handle_t *handle, uint16_t size) {
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
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL || Size == 0 || handle->config.mode == UART_MODE_BLOCKING) {
        return;
    }

    CompleteReception(handle, Size);
}

/**
 * @brief Transmission finished
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL) {
        return;
    }

    handle->txComplete = true;
}

/**
 * @brief Requested number of bytes received
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL || handle->config.mode == UART_MODE_BLOCKING) {
        return;
    }

    /* This callback only fires once rxBuffer is full, in both async modes. */
    CompleteReception(handle, handle->rxSize);
}

/**
 * @brief Line error: clear the cause, then get reception running again
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
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

    if (handle->config.mode == UART_MODE_INTERRUPT) {
        /* Nothing here can act on a failure, but silently losing the port is
           worse than saying so. */
        if (!UART_Interrupt_Recover(handle)) {
            log_error("UART: recovery failed; reception stays down");
        }
    }

    RestartReceive(handle);
}
