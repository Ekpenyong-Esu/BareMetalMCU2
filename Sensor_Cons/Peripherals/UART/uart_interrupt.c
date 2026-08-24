/**
 * @file uart_interrupt.c
 * @brief Interrupt-driven transfer mode
 */

#include "uart_interrupt.h"
#include "uart.h"
#include "uart_config.h"
#include "log.h"
#include <string.h>

/* Reception is armed the same way whether it is the first call or a re-arm
   from interrupt context, so both paths share this. Bytes always land in the
   driver's landing buffer; the caller's buffer is filled from the ring. */
static bool StartReceive(UART_Handle_t* handle)
{
    return HAL_UARTEx_ReceiveToIdle_IT(handle->huart, handle->rxBuffer, handle->rxSize) == HAL_OK;
}

/* The ISR fills the ring while the caller drains it, and head/tail/count are
   plain read-modify-write. Masking interrupts for the length of one copy is
   cheap next to an 87us byte time at 115200 baud. */
static uint16_t ReadRing(UART_Handle_t* handle, uint8_t* data, uint16_t size)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    uint32_t read = RingBuffer_Read(&handle->rxRing, data, size);

    __set_PRIMASK(primask);
    return (uint16_t)read;
}

UART_Status_t UART_Interrupt_Init(UART_Handle_t* handle, const UART_Config_t* config,
                                  uint8_t* rxBuffer, uint16_t rxBufferSize)
{
    if (handle == NULL || config == NULL || config->instance == NULL || handle->huart == NULL) {
        log_debug("Interrupt UART: handle, huart or config is NULL");
        return UART_ERROR;
    }

    if (rxBuffer == NULL || rxBufferSize == 0) {
        log_debug("Interrupt UART: no RX landing buffer");
        return UART_ERROR;
    }

    /* A chunk larger than the ring could never be stored whole. */
    if (rxBufferSize > RING_BUFFER_SIZE) {
        log_debug("Interrupt UART: RX buffer larger than the ring");
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
        log_debug("Interrupt UART initialization failed");
        return UART_ERROR;
    }

    /* IDLE marks the end of a frame; ERR surfaces overrun and framing faults. */
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR);

    handle->isInitialized = true;

    /* Listen from now on, not from the next Read(), so bytes arriving while the
       main loop is busy are captured instead of dropped. */
    if (!StartReceive(handle)) {
        log_debug("Interrupt UART failed to start listening");
        handle->isInitialized = false;
        return UART_ERROR;
    }

    log_debug("UART: opened in interrupt mode");
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

UART_Status_t UART_Interrupt_Read(UART_Handle_t* handle, uint8_t* data, uint16_t size,
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

UART_Status_t UART_Interrupt_Write(UART_Handle_t* handle, const uint8_t* data, uint16_t size)
{
    if (!IsReadyForTransfer(handle, data, size)) {
        return UART_ERROR;
    }

    /* One send at a time: there is no TX queue, the caller's buffer is the queue. */
    if (!handle->txComplete) {
        return UART_BUSY;
    }

    handle->txComplete = false;

    if (HAL_UART_Transmit_IT(handle->huart, (uint8_t*)data, size) != HAL_OK) {
        handle->txComplete = true;
        log_debug("UART Transmit failed to start");
        return UART_ERROR;
    }

    return UART_OK;
}

bool UART_Interrupt_IsTxDone(const UART_Handle_t* handle)
{
    if (handle == NULL) {
        return false;
    }

    return handle->txComplete;
}

void UART_Interrupt_Rearm(UART_Handle_t* handle)
{
    StartReceive(handle);
}

void UART_Interrupt_Recover(UART_Handle_t* handle)
{
    /* Interrupt mode needs the peripheral rebuilt before reception can restart;
       HAL_UART_Init() rewrites CR1/CR3, so the interrupts go back on after it. */
    HAL_UART_Init(handle->huart);
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR);

    /* HAL_UART_Init() reset gState, so a send that was in flight is gone and
       its TxCplt callback will never arrive. Release the caller's buffer here
       or IsTxDone() stays false forever and the loop above it never runs again. */
    handle->txComplete = true;
}

/*
 * The UART interrupt vector is owned by Core, not this driver.
 * USART1_IRQHandler() is defined in Core/Src/stm32f4xx_it.c and dispatches
 * into the callbacks in uart_events.c via HAL_UART_IRQHandler().
 */
