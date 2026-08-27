/**
 * @file uart_events.c
 * @brief Functions HAL calls automatically when something happens on UART
 *
 * These are "callbacks" - HAL runs them for us when an interrupt fires
 * (data arrived, send finished, error occurred). We never call them
 * ourselves. All of this runs inside an interrupt, so keep it fast.
 */

#include "uart.h"
#include "uart_interrupt.h"
#include "uart_dma.h"
#include "log.h"

/* HAL only tells us which raw huart triggered the event, not our own handle.
 * Look up our handle from it, so we can ignore events from other UARTs. */
static UART_Handle_t *OwnerOf(const UART_HandleTypeDef *huart)
{
    UART_Handle_t *handle = UART_GetActiveHandle();

    return (handle != NULL && handle->isInitialized && handle->huart == huart) ? handle : NULL;
}

/* Once a receive finishes, HAL stops listening until we tell it to start
 * again - so we "re-arm" it here. Blocking mode has nothing to re-arm. */
static void RestartReceive(UART_Handle_t *handle)
{
    switch (handle->config.mode) {
        case UART_MODE_INTERRUPT: UART_Interrupt_Rearm(handle); break;
        case UART_MODE_DMA:       UART_DMA_Rearm(handle); break;
        default:                  break;
    }
}

/* Called whenever new bytes have landed in rxBuffer, no matter why. */
static void CompleteReception(UART_Handle_t *handle, uint16_t size)
{
    if (size > 0) {
        /* Copy the fresh bytes into the ring buffer so they're safe before*/
        uint32_t stored = RingBuffer_PutBytes(&handle->rxRing, handle->rxBuffer, size); // We copy the data from the HAL landing buffer into our ring buffer, which is where the main program will read it from.
        if (stored < size) {
            log_debug("UART RX ring full, dropped %u bytes", (unsigned)(size - stored));
        }
    }

    handle->rxComplete = true;

    RestartReceive(handle); /* start listening and receiving into the HAL landing buffer */
}

/**
 * @brief Called when the line goes idle, meaning the sender paused/stopped
 *        and whatever arrived so far (maybe less than a full buffer) is ready
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL || Size == 0 || handle->config.mode == UART_MODE_BLOCKING) {
        return;
    }

    CompleteReception(handle, Size);
}

/**
 * @brief Called once all the bytes we asked to send have gone out
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
 * @brief Called when rxBuffer has completely filled up (no idle pause needed)
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL || handle->config.mode == UART_MODE_BLOCKING) {
        return;
    }

    /* Buffer is full, so the whole thing counts as the received data. */
    CompleteReception(handle, handle->rxSize);
}

/**
 * @brief Called when something went wrong on the line (noise, overrun, etc.)
 *        Logs what happened, clears the error, then starts listening again
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    UART_Handle_t *handle = OwnerOf(huart);

    if (handle == NULL) {
        return;
    }

    /* Each error has its own flag; check and clear them one at a time. */
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

    /* Cancel whatever receive was in progress before restarting it,
     * otherwise the error keeps coming back. */
    HAL_UART_AbortReceive(huart);
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    if (handle->config.mode == UART_MODE_INTERRUPT) {
        UART_Interrupt_Recover(handle);
    }

    RestartReceive(handle);

    log_debug("UART error cleared, reception restarted");
}
