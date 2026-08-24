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

/* Reception is armed the same way whether it is the first call or a re-arm
   from interrupt context, so both paths share this. Bytes always land in the
   driver's landing buffer; the caller's buffer is filled from the ring. */
static bool StartReceive(UART_Handle_t* handle)
{
    if (HAL_UARTEx_ReceiveToIdle_DMA(handle->huart, handle->rxBuffer, handle->rxSize) != HAL_OK) {
        return false;
    }

    /* Half-transfer interrupts would report packets that have not arrived yet. */
    __HAL_DMA_DISABLE_IT(handle->huart->hdmarx, DMA_IT_HT);
    return true;
}

/* The DMA completion ISR fills the ring while the caller drains it, and
   head/tail/count are plain read-modify-write. Masking interrupts for the
   length of one copy is cheap next to an 87us byte time at 115200 baud. */
static uint16_t ReadRing(UART_Handle_t* handle, uint8_t* data, uint16_t size)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint32_t read = RingBuffer_Read(&handle->rxRing, data, size);

    __set_PRIMASK(primask);
    return (uint16_t)read;
}

// Open a link in DMA mode and start listening immediately.
UART_Status_t UART_DMA_Init(UART_Handle_t* handle, const UART_Config_t* config,
                            uint8_t* rxBuffer, uint16_t rxBufferSize)
{
    if (handle == NULL || config == NULL || config->instance == NULL || handle->huart == NULL) {
        log_debug("DMA UART: handle, huart or config is NULL");
        return UART_ERROR;
    }

    if (rxBuffer == NULL || rxBufferSize == 0) {
        log_debug("DMA UART: no RX landing buffer");
        return UART_ERROR;
    }

    /* A chunk larger than the ring could never be stored whole. */
    if (rxBufferSize > RING_BUFFER_SIZE) {
        log_debug("DMA UART: RX buffer larger than the ring");
        return UART_ERROR;
    }

    if (handle->isInitialized) {
        UART_DeInit(handle);
    }

    handle->config = *config;
    handle->rxBuffer = rxBuffer;
    handle->rxSize = rxBufferSize;
    handle->txComplete = true; /* idle counts as done, so the first Write() may start */
    handle->rxComplete = false;
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

    /* Listen from now on, not from the next Read(), so bytes arriving while the
       main loop is busy are captured instead of dropped. */
    if (!StartReceive(handle)) {
        log_debug("DMA UART failed to start listening");
        handle->isInitialized = false;
        return UART_ERROR;
    }

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

// Take whatever has been received so far, up to size bytes; returns at once.
UART_Status_t UART_DMA_Read(UART_Handle_t* handle, uint8_t* data, uint16_t size,
                            uint16_t* received)
{
    if (received == NULL) {
        log_debug("UART received-count pointer is NULL");
        return UART_ERROR;
    }

    *received = 0;

    if (!IsReadyForTransfer(handle, data, size)) {
        return UART_ERROR;
    }

    /* Reception is already running; this only moves bytes out of the ring. */
    *received = ReadRing(handle, data, size);
    return UART_OK;
}

// Start sending and return; the DMA stream moves the bytes out.
UART_Status_t UART_DMA_Write(UART_Handle_t* handle, const uint8_t* data, uint16_t size)
{
    if (!IsReadyForTransfer(handle, data, size)) {
        return UART_ERROR;
    }

    /* One send at a time: there is no TX queue, the caller's buffer is the queue. */
    if (!handle->txComplete) {
        return UART_BUSY;
    }

    handle->txComplete = false;

    /* The stream reads straight out of the caller's memory, which is why the
       buffer has to stay put until IsTxDone(). No cache maintenance is needed:
       the Cortex-M4 on this part has no data cache for DMA to go stale against. */
    if (HAL_UART_Transmit_DMA(handle->huart, (uint8_t*)data, size) != HAL_OK) {
        handle->txComplete = true;
        log_debug("DMA UART Transmit failed to start");
        return UART_ERROR;
    }

    return UART_OK;
}

// True once the last Write() has finished leaving the peripheral.
bool UART_DMA_IsTxDone(const UART_Handle_t* handle)
{
    if (handle == NULL) {
        return false;
    }

    return handle->txComplete;
}

// Re-arm reception after a completed transfer.
void UART_DMA_Rearm(UART_Handle_t* handle)
{
    StartReceive(handle);
}

void UART_DMA_Recover(UART_Handle_t* handle)
{
    /* The error path aborts the streams, so a send that was in flight is gone
       and its TxCplt callback will never arrive. Release the caller's buffer
       here or IsTxDone() stays false forever and the loop above it stalls. */
    handle->txComplete = true;
}

/*
 * DMA interrupt vectors are owned by Core, not this driver.
 * DMA2_Stream7_IRQHandler() (TX) and DMA2_Stream5_IRQHandler() (RX) are
 * defined in Core/Src/stm32f4xx_it.c and dispatch via HAL_DMA_IRQHandler().
 * The USART1 vector still matters too: IDLE is a UART event, not a DMA one.
 */
