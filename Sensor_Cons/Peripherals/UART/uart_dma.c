/**
 * @file uart_dma.c
 * @brief DMA mode: a separate piece of hardware moves the bytes for us
 *
 * DMA (Direct Memory Access) copies bytes between the UART and memory
 * without the CPU touching each one. This frees up the CPU the most out
 * of all three modes, at the cost of needing DMA streams to be set up.
 */

#include "uart_dma.h"
#include "uart.h"
#include "uart_config.h"
#include "stm32f4xx_hal_dma.h"
#include "log.h"
#include <string.h>

/* Blocks until a callback sets *flag true, or timeout elapses. */
static UART_Status_t WaitForFlag(volatile bool *flag, uint32_t timeout)
{
    uint32_t startTick = HAL_GetTick();

    while (!*flag) {
        if ((HAL_GetTick() - startTick) > timeout) {
            return UART_TIMEOUT_ERROR;
        }
    }

    return UART_OK;
}

/* Polls the ring buffer for `size` bytes until they arrive or we time out. */
static UART_Status_t WaitForRingData(UART_Handle_t *handle, uint8_t *data, uint16_t size, uint32_t timeout)
{
    uint32_t startTick = HAL_GetTick();

    while (timeout > 0 && !handle->rxComplete) {
        if (RingBuffer_GetBytes(&handle->rxRing, data, size)) {
            return UART_OK;
        }

        if ((HAL_GetTick() - startTick) > timeout) {
            log_debug("UART Receive timeout");
            return UART_TIMEOUT_ERROR;
        }
    }

    if (RingBuffer_GetBytes(&handle->rxRing, data, size)) {
        return UART_OK;
    }

    return UART_ERROR;
}

/* Arms DMA reception (idle-line or fixed-length); shared by first receive and every re-arm. */
static bool StartReceive(UART_Handle_t* handle)
{
    if (HAL_UARTEx_ReceiveToIdle_DMA(handle->huart, handle->rxBuffer, handle->rxSize) == HAL_OK) {
        /* Half-full interrupt is noise - we only care once the packet finishes. */
        __HAL_DMA_DISABLE_IT(handle->huart->hdmarx, DMA_IT_HT);
        return true;
    }

    log_debug("DMA ReceiveToIdle failed, falling back to fixed-length receive");
    return HAL_UART_Receive_DMA(handle->huart, handle->rxBuffer, handle->rxSize) == HAL_OK;
}

UART_Status_t UART_DMA_Init(UART_Handle_t* handle, const UART_Config_t* config)
{
    if (handle == NULL || config == NULL || config->instance == NULL || handle->huart == NULL) {
        log_debug("DMA UART: handle, huart or config is NULL");
        return UART_ERROR;
    }

    if (handle->isInitialized) {
        UART_DeInit(handle);
    }

    handle->config = *config;

    RingBuffer_Init(&handle->rxRing);
    
    memset(handle->huart, 0, sizeof(UART_HandleTypeDef));

    /* Must be set before HAL_UART_Init() calls HAL_UART_MspInit(), which needs it to wire up DMA. */
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
        log_debug("DMA UART initialization failed");
        return UART_ERROR;
    }

    /* Fail now rather than crash later on an unwired DMA stream. */
    if (handle->huart->hdmatx == NULL || handle->huart->hdmarx == NULL) {
        log_error("UART: DMA mode selected but no DMA stream is linked");
        return UART_ERROR;
    }

    /* IDLE tells us a packet ended; DMA (not RXNE) handles moving the bytes. */
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE);

    handle->isInitialized = true;
    log_debug("UART: opened in DMA mode");
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

UART_Status_t UART_DMA_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (!IsReadyForTransfer(handle, data, size)) {
        return UART_ERROR;
    }

    handle->txComplete = false;

    if (HAL_UART_Transmit_DMA(handle->huart, (uint8_t*)data, size) != HAL_OK) {
        log_debug("DMA UART Transmit failed");
        return UART_ERROR;
    }

    if (timeout == 0) {
        return UART_OK;
    }

    return WaitForFlag(&handle->txComplete, timeout);
}

UART_Status_t UART_DMA_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (!IsReadyForTransfer(handle, data, size)) {
        return UART_ERROR;
    }

    if (size > RING_BUFFER_SIZE) {
        log_debug("Requested size exceeds ring buffer size");
        return UART_ERROR;
    }

    handle->rxComplete = false;

    if (!StartReceive(handle)) {
        log_debug("DMA UART Receive failed to start");
        return UART_ERROR;
    }

    /* A callback copies DMA's rxBuffer into the ring, so we wait on the ring, not rxBuffer. */
    return WaitForRingData(handle, data, size, timeout);
}

/* Called after a receive finishes, to start listening for the next one. */
void UART_DMA_Rearm(UART_Handle_t* handle)
{
    StartReceive(handle);
}

/* DMA2_Stream7/5_IRQHandler() (TX/RX) live in Core/Src/stm32f4xx_it.c, not here. */
