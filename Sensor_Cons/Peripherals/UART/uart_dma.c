/**
 * @file uart_dma.c
 * @brief DMA transfer mode
 */

#include "uart_dma.h"
#include "uart_core.h"
#include "stm32f4xx_hal_dma.h"
#include "log.h"

/* Reception is armed the same way whether it is the first call or a re-arm
   from interrupt context, so both paths share this. */
static bool StartReceive(UART_Handle_t* handle)
{
    if (HAL_UARTEx_ReceiveToIdle_DMA(handle->huart, handle->rxBuffer, handle->rxSize) == HAL_OK) {
        /* Half-transfer interrupts would report packets that have not arrived yet. */
        __HAL_DMA_DISABLE_IT(handle->huart->hdmarx, DMA_IT_HT);
        return true;
    }

    log_debug("DMA ReceiveToIdle failed, falling back to fixed-length receive");
    return HAL_UART_Receive_DMA(handle->huart, handle->rxBuffer, handle->rxSize) == HAL_OK;
}

static UART_Status_t Init(UART_Handle_t* handle)
{
    /* MspInit is what links the DMA streams. Without them HAL_UART_*_DMA would
       dereference a NULL hdmatx/hdmarx on the first transfer. */
    if (handle->huart->hdmatx == NULL || handle->huart->hdmarx == NULL) {
        log_error("UART: DMA mode selected but no DMA stream is linked");
        return UART_ERROR;
    }

    /* IDLE detects packet boundaries; RXNE would fight the DMA controller. */
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE);

    return UART_OK;
}

static UART_Status_t Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    handle->txComplete = false;

    if (HAL_UART_Transmit_DMA(handle->huart, (uint8_t*)data, size) != HAL_OK) {
        log_debug("DMA UART Transmit failed");
        return UART_ERROR;
    }

    if (timeout == 0) {
        return UART_OK;
    }

    return UART_WaitForFlag(&handle->txComplete, timeout);
}

static UART_Status_t Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (size > RING_BUFFER_SIZE) {
        log_debug("Requested size exceeds ring buffer size");
        return UART_ERROR;
    }

    handle->rxComplete = false;

    if (!StartReceive(handle)) {
        log_debug("DMA UART Receive failed to start");
        return UART_ERROR;
    }

    /* DMA lands in handle->rxBuffer and the callbacks forward it into the ring,
       so the caller's buffer is only filled by draining the ring. */
    uint32_t startTick = HAL_GetTick();
    while (timeout > 0 && !handle->rxComplete) {
        if (RingBuffer_GetBytes(&handle->rxRing, data, size)) {
            return UART_OK;
        }

        if ((HAL_GetTick() - startTick) > timeout) {
            log_debug("DMA UART Receive timeout");
            return UART_TIMEOUT_ERROR;
        }
    }

    return RingBuffer_GetBytes(&handle->rxRing, data, size) ? UART_OK : UART_ERROR;
}

static void RearmReceive(UART_Handle_t* handle)
{
    StartReceive(handle);
}

static uint16_t CompletedReceiveSize(const UART_Handle_t* handle, const UART_HandleTypeDef* huart)
{
    (void)huart;

    /* This callback only fires once rxBuffer is full. */
    return handle->rxSize;
}

const UART_ModeOps_t UART_DmaOps = {
    .name = "DMA",
    .init = Init,
    .transmit = Transmit,
    .receive = Receive,
    .rearmReceive = RearmReceive,
    .completedReceiveSize = CompletedReceiveSize,
    .recoverFromError = NULL,
};

/*
 * DMA interrupt vectors are owned by Core, not this driver.
 * DMA2_Stream7_IRQHandler() (TX) and DMA2_Stream5_IRQHandler() (RX) are
 * defined in Core/Src/stm32f4xx_it.c and dispatch via HAL_DMA_IRQHandler().
 */
