/**
 * @file uart_dma.c
 * @brief DMA transfer mode
 */

#include "uart_dma.h"
#include "uart.h"
#include "uart_config.h"
#include "stm32f4xx_hal_dma.h"
#include "log.h"
#include <string.h>

/* Spins until the TX callback raises txComplete; self-contained, so this mode
   never depends on the driver's other files. */
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

/* Reception lands in the ring; this drains a complete packet out of it. */
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

    /* Publish before HAL_UART_Init(), which calls into HAL_UART_MspInit() and
     * needs the mode to decide whether to wire up DMA. */
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

    /* MspInit is what links the DMA streams. Without them HAL_UART_*_DMA would
       dereference a NULL hdmatx/hdmarx on the first transfer. */
    if (handle->huart->hdmatx == NULL || handle->huart->hdmarx == NULL) {
        log_error("UART: DMA mode selected but no DMA stream is linked");
        return UART_ERROR;
    }

    /* IDLE detects packet boundaries; RXNE would fight the DMA controller. */
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

    /* DMA lands in handle->rxBuffer and the callbacks forward it into the ring,
       so the caller's buffer is only filled by draining the ring. */
    return WaitForRingData(handle, data, size, timeout);
}

void UART_DMA_Rearm(UART_Handle_t* handle)
{
    StartReceive(handle);
}

/*
 * DMA interrupt vectors are owned by Core, not this driver.
 * DMA2_Stream7_IRQHandler() (TX) and DMA2_Stream5_IRQHandler() (RX) are
 * defined in Core/Src/stm32f4xx_it.c and dispatch via HAL_DMA_IRQHandler().
 */
