/**
 * @file uart.c
 * @brief UART lifecycle and mode-agnostic transfers
 *
 * Owns opening and closing a link and routing transfers to the configured
 * mode. Interrupt-context work lives in uart_events.c; board wiring lives in
 * Core/Src/stm32f4xx_hal_msp.c.
 */

#include "uart_core.h"

#include "uart_config.h"
#include "uart_dma.h"
#include "uart_interrupt.h"
#include "uart_blocking.h"
#include "log.h"
#include <string.h>

/* The MSP and the interrupt vectors run without a caller-supplied handle, so
 * the driver publishes the link it is currently serving. */
static UART_Handle_t *s_activeHandle = NULL;

UART_Handle_t *UART_GetActiveHandle(void)
{
    return s_activeHandle;
}

UART_Status_t UART_WaitForFlag(volatile bool *flag, uint32_t timeout)
{
    uint32_t startTick = HAL_GetTick();

    while (!*flag) {
        if ((HAL_GetTick() - startTick) > timeout) {
            return UART_TIMEOUT_ERROR;
        }
    }

    return UART_OK;
}

/* The one place that maps a mode onto an implementation. */
static const UART_ModeOps_t *OpsFor(UART_Mode_t mode)
{
    switch (mode) {
        case UART_MODE_BLOCKING:  return &UART_BlockingOps;
        case UART_MODE_INTERRUPT: return &UART_InterruptOps;
        case UART_MODE_DMA:       return &UART_DmaOps;
        default:                  return NULL;
    }
}

UART_Status_t UART_Init(UART_Handle_t* handle, const UART_Config_t* config)
{
    if (handle == NULL || config == NULL || config->instance == NULL) {
        log_debug("UART handle or config is NULL");
        return UART_ERROR;
    }

    if (handle->huart == NULL) {
        log_debug("UART HAL handle is NULL");
        return UART_ERROR;
    }

    const UART_ModeOps_t *ops = OpsFor(config->mode);
    if (ops == NULL) {
        log_debug("Invalid UART mode");
        return UART_ERROR;
    }

    log_debug("UART: opening link in %s mode", ops->name);

    /* Deinitialize previous configuration if initialized */
    if (handle->isInitialized) {
        UART_DeInit(handle);
    }

    handle->config = *config;
    handle->ops = ops;
    handle->txComplete = false;
    handle->rxComplete = false;
    RingBuffer_Init(&handle->rxRing);

    memset(handle->huart, 0, sizeof(UART_HandleTypeDef));

    /* Publish before HAL_UART_Init(), which calls into HAL_UART_MspInit() and
     * needs the mode to decide whether to wire up DMA. */
    s_activeHandle = handle;

    handle->huart->Instance = config->instance;
    handle->huart->Init.BaudRate = config->baudRate;
    handle->huart->Init.WordLength = config->wordLength;
    handle->huart->Init.StopBits = config->stopBits;
    handle->huart->Init.Parity = config->parity;
    handle->huart->Init.Mode = UART_DEFAULT_MODE;
    handle->huart->Init.HwFlowCtl = UART_HWCONTROL_NONE;
    handle->huart->Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(handle->huart) != HAL_OK) {
        log_debug("UART initialization failed");
        return UART_ERROR;
    }

    UART_Status_t status = ops->init(handle);
    if (status == UART_OK) {
        handle->isInitialized = true;
        log_debug("UART: UART initialized successfully");
    }

    return status;
}

UART_Status_t UART_DeInit(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        log_debug("UART handle or huart is NULL");
        return UART_ERROR;
    }

    /* Disable all UART interrupts */
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_RXNE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_TC);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_PE);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_ERR);
    __HAL_UART_DISABLE_IT(handle->huart, UART_IT_IDLE);

    /* HAL_UART_DeInit() releases the MSP resources on our behalf. */
    if (HAL_UART_DeInit(handle->huart) != HAL_OK) {
        log_debug("UART deinitialization failed");
        return UART_ERROR;
    }

    /* Reset handle state */
    handle->isInitialized = false;
    handle->ops = NULL;
    handle->rxBuffer = NULL;
    handle->txBuffer = NULL;
    handle->rxSize = 0;
    handle->txComplete = false;
    handle->rxComplete = false;
    memset(&handle->config, 0, sizeof(UART_Config_t));

    if (s_activeHandle == handle) {
        s_activeHandle = NULL;
    }

    return UART_OK;
}

/* Both transfer directions share the same preconditions. */
static bool IsReadyForTransfer(const UART_Handle_t *handle, const void *data, uint16_t size)
{
    if (handle == NULL || data == NULL || size == 0) {
        log_debug("UART handle, data is NULL or size is 0");
        return false;
    }

    if (!handle->isInitialized || handle->ops == NULL) {
        log_debug("UART not initialized");
        return false;
    }

    return true;
}

UART_Status_t UART_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (!IsReadyForTransfer(handle, data, size)) {
        return UART_ERROR;
    }

    return handle->ops->transmit(handle, data, size, timeout);
}

UART_Status_t UART_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (!IsReadyForTransfer(handle, data, size)) {
        return UART_ERROR;
    }

    return handle->ops->receive(handle, data, size, timeout);
}

