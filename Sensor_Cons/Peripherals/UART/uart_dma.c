/**
 * @file uart_dma.c
 * @brief UART DMA mode implementation for STM32F429I-DISC1
 */

#include "uart_dma.h"
#include "uart_config.h"
#include "stm32f4xx_hal_dma.h"
#include "log.h"

/* External references */
extern UART_Handle_t uartHandle;  // Define this in uart.c

UART_Status_t UART_DMA_Init(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        log_debug("DMA UART handle or huart is NULL");
        return UART_ERROR;
    }

    /* MspInit is what links the DMA streams. Without them HAL_UART_*_DMA would
       dereference a NULL hdmatx/hdmarx on the first transfer. */
    if (handle->huart->hdmatx == NULL || handle->huart->hdmarx == NULL) {
        log_error("UART: DMA mode selected but no DMA stream is linked");
        return UART_ERROR;
    }

    /* Note: DMA hardware initialization is now handled in HAL_UART_MspInit() */
    /* This function only configures the application-level settings */

    /* For DMA mode, only enable IDLE interrupt for packet detection */
    /* Do NOT enable RXNE interrupt as it conflicts with DMA */
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR);  // Keep error interrupt
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE); // Keep IDLE line detection

    /* Initialize ring buffer first */
    UART_RingBuffer_Init();

    handle->isInitialized = true;
    return UART_OK;
}


UART_Status_t UART_DMA_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        log_debug("DMA UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    /* Reset transmission complete flag */
    txComplete = 0;

    HAL_StatusTypeDef status = HAL_UART_Transmit_DMA(handle->huart, data, size);
    if (status != HAL_OK) {
        log_debug("DMA UART Transmit failed");
        return UART_ERROR;
    }

     /* Wait for transmission complete if timeout is specified */
    if (timeout > 0) {
        uint32_t startTick = HAL_GetTick();
        while (!txComplete) {
            if ((HAL_GetTick() - startTick) > timeout) {
                log_debug("DMA UART Transmit timeout");
                return UART_TIMEOUT_ERROR;
            }
        }
    }

    return UART_OK;
}

UART_Status_t UART_DMA_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        log_debug("DMA UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    if (size > RING_BUFFER_SIZE) {
        log_debug("Requested size exceeds ring buffer size");
        return UART_ERROR;
    }

    /* Reset reception complete flag */
    rxComplete = 0;

    /* Use IDLE line detection for more responsive reception */
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_DMA(handle->huart, handle->rxBuffer, handle->rxSize);
    if (status != HAL_OK) {
        log_debug("DMA UART ReceiveToIdle failed: %d", status);
        /* Fallback to regular DMA receive if ReceiveToIdle fails */
        status = HAL_UART_Receive_DMA(handle->huart, handle->rxBuffer, handle->rxSize);
        if (status != HAL_OK) {
            log_debug("DMA UART Receive failed: %d", status);
            return UART_ERROR;
        }
    }

    /* Disable DMA Half Transfer interrupt to avoid conflicts */
    __HAL_DMA_DISABLE_IT(handle->huart->hdmarx, DMA_IT_HT);

    /* If timeout is specified, wait for completion or timeout */
    if (timeout > 0) {
        uint32_t startTick = HAL_GetTick();
        while (!rxComplete) {
            if ((HAL_GetTick() - startTick) > timeout) {
                log_debug("DMA UART Receive timeout");
                return UART_TIMEOUT_ERROR;
            }

            /* Check if data is available in ring buffer */
            if (UART_RingBuffer_Receive(handle, data, size) == UART_OK) {
                return UART_OK;
            }
        }
    }

    /* DMA lands in handle->rxBuffer, so the caller's buffer is only filled by
       draining the ring buffer. Without this the function reported UART_OK
       while leaving data untouched. */
    return UART_RingBuffer_Receive(handle, data, size);
}

/*
 * DMA interrupt vectors are owned by the interrupt layer, not this driver.
 * DMA2_Stream7_IRQHandler() (TX) and DMA2_Stream5_IRQHandler() (RX) are
 * defined in Core/Src/stm32f4xx_it.c and dispatch via HAL_DMA_IRQHandler().
 */
