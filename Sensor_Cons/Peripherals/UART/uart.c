/**
 * @file uart.c
 * @brief Shared UART utilities: teardown, instance registry, vector dispatch, flag waits
 *
 * Each transfer mode owns its own Init/Transmit/Receive; this file never
 * includes a mode header and has no dispatch table, matching how tim_clock.c
 * knows nothing about tim_pwm.c or tim_ic.c.
 *
 * Instance Registry:
 * - HAL_UART_MspInit(), the HAL callbacks and the interrupt vectors all run
 *   without a caller-supplied handle
 * - Each mode's Init() calls UART_Register() before HAL_UART_Init(), and
 *   UART_DeInit() removes the entry once the MSP no longer needs it
 * - One slot per USART instance, so links on different instances coexist
 * - UART_FromInstance() is the only reader
 *
 * Flag Waiting:
 * - UART_WaitForFlag() busy-waits on a volatile boolean
 * - Used by interrupt/DMA modes to wait for txComplete/rxComplete flags
 * - Timeout uses HAL_GetTick() for millisecond resolution
 */

#include "uart.h"

#include "log.h"
#include <string.h>

/** @brief Number of USART/UART instances on the STM32F429 */
#define UART_INSTANCE_COUNT 6

/** @brief Instance index to owning handle; the only file-scope state here */
static UART_Handle_t *s_registry[UART_INSTANCE_COUNT] = {NULL};

/**
 * @brief Map a USART instance pointer to a registry index
 * @param instance USART1..USART6 / UART4 / UART5
 * @retval int 0..5, or -1 for an unknown instance
 */
static int UART_InstanceIndex(const USART_TypeDef *instance) {
    static USART_TypeDef *const instances[UART_INSTANCE_COUNT] = {
        USART1, USART2, USART3, UART4, UART5, USART6,
    };

    for (int i = 0; i < UART_INSTANCE_COUNT; i++) {
        if (instances[i] == instance) {
            return i;
        }
    }

    return -1;
}

/* Refuses a binding the MSP could not act on, so the failure surfaces at
   Init() rather than as a silent pin left unconfigured. */
static bool UART_ConfigIsBound(const UART_Config_t *config) {
    if (config->txPort == NULL && config->rxPort == NULL) {
        log_error("UART: neither TX nor RX pin given");
        return false;
    }

    if ((config->txPort != NULL && config->txPin == 0U) ||
        (config->rxPort != NULL && config->rxPin == 0U)) {
        log_error("UART: pin port given without a pin");
        return false;
    }

    if (config->mode == UART_MODE_DMA &&
        (config->dmaTxStream == NULL || config->dmaRxStream == NULL)) {
        log_error("UART: DMA mode needs both a TX and an RX stream");
        return false;
    }

    return true;
}

UART_Handle_t *UART_FromInstance(const USART_TypeDef *instance) {
    int index = UART_InstanceIndex(instance);

    return (index < 0) ? NULL : s_registry[index];
}

UART_Status_t UART_Register(UART_Handle_t *handle) {
    int index = -1;

    if (handle == NULL) {
        return UART_ERROR;
    }

    index = UART_InstanceIndex(handle->config.instance);
    if (index < 0) {
        log_error("UART: unknown USART instance");
        return UART_ERROR;
    }

    if (!UART_ConfigIsBound(&handle->config)) {
        return UART_ERROR;
    }

    if (s_registry[index] != NULL && s_registry[index] != handle) {
        log_error("UART: instance already open on another handle");
        return UART_ERROR;
    }

    s_registry[index] = handle;
    return UART_OK;
}

void UART_Unregister(UART_Handle_t *handle) {
    for (int i = 0; i < UART_INSTANCE_COUNT; i++) {
        if (s_registry[i] == handle) {
            s_registry[i] = NULL;
        }
    }
}

void UART_IRQHandler(USART_TypeDef *instance) {
    UART_Handle_t *handle = UART_FromInstance(instance);

    if (handle != NULL && handle->huart != NULL) {
        HAL_UART_IRQHandler(handle->huart);
    }
}

/* The vector names a stream, not a UART, so the owner has to be found by
   comparing the streams each open link was given. */
void UART_DmaStreamIRQHandler(const DMA_Stream_TypeDef *stream) {
    for (int i = 0; i < UART_INSTANCE_COUNT; i++) {
        UART_Handle_t *handle = s_registry[i];

        if (handle == NULL || handle->huart == NULL) {
            continue;
        }

        if (handle->huart->hdmatx != NULL && handle->huart->hdmatx->Instance == stream) {
            HAL_DMA_IRQHandler(handle->huart->hdmatx);
            return;
        }

        if (handle->huart->hdmarx != NULL && handle->huart->hdmarx->Instance == stream) {
            HAL_DMA_IRQHandler(handle->huart->hdmarx);
            return;
        }
    }
}

/**
 * @brief Spin until a completion flag is raised
 *
 * Busy-waits on a volatile boolean flag (typically txComplete or rxComplete
 * set by interrupt callbacks). Uses HAL_GetTick() for timeout.
 *
 * @param flag    Pointer to volatile flag that an interrupt callback will set
 * @param timeout Milliseconds to wait; 0 returns UART_TIMEOUT_ERROR unless
 *                the flag is already raised
 * @retval UART_OK once raised, UART_TIMEOUT_ERROR if the wait expired
 */
UART_Status_t UART_WaitForFlag(const volatile bool *flag, uint32_t timeout) {
    uint32_t startTick = HAL_GetTick();

    while (!*flag) {
        if ((HAL_GetTick() - startTick) > timeout) {
            return UART_TIMEOUT_ERROR;
        }
    }

    return UART_OK;
}

/**
 * @brief Close a UART link and release its hardware resources
 *
 * Disables all UART interrupts (RXNE, TC, PE, ERR, IDLE), calls
 * HAL_UART_DeInit() to release MSP resources (GPIO, clocks, DMA), removes
 * the link from the registry and clears the handle state.
 *
 * @param handle UART handle to deinitialize
 * @retval UART_OK on success, UART_ERROR if handle/huart is NULL or HAL fails
 */
UART_Status_t UART_DeInit(UART_Handle_t *handle) {
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

    /* HAL_UART_DeInit() releases the MSP resources on our behalf; the MSP
       still needs the registry entry to find the pins and streams. */
    if (HAL_UART_DeInit(handle->huart) != HAL_OK) {
        log_debug("UART deinitialization failed");
        return UART_ERROR;
    }

    UART_Unregister(handle);

    /* Reset handle state */
    handle->isInitialized = false;
    handle->rxBuffer = NULL;
    handle->rxSize = 0;
    handle->txComplete = false;
    handle->rxComplete = false;
    memset(&handle->config, 0, sizeof(UART_Config_t));

    return UART_OK;
}
