/**
 * @file uart.c
 * @brief Main UART implementation for STM32F429I-DISC1
 */

#include "uart.h"
#include "uart_config.h"
#include "uart_dma.h"
#include "uart_interrupt.h"
#include "uart_blocking.h"
#include "sys.h"
#include "gpio.h"
#include "log.h"
#include <string.h>

/* Ensure uartHandle is declared globally */
extern UART_Handle_t uartHandle;

/* Transfer completion flags. They live here rather than in uart_example.c
   because the HAL callbacks below are what set them, and the driver must link
   without the example. */
volatile uint8_t txComplete = 0;
volatile uint8_t rxComplete = 0;

/* Re-arms reception for the configured mode; the plain HAL call is the fallback
   when ReceiveToIdle is refused. */
static void UART_RestartReceive(UART_HandleTypeDef* huart)
{
    if (uartHandle.config.mode == UART_MODE_DMA) {
        if (HAL_UARTEx_ReceiveToIdle_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize) != HAL_OK) {
            log_debug("Failed to restart DMA ReceiveToIdle");
            HAL_UART_Receive_DMA(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        }
    } else if (uartHandle.config.mode == UART_MODE_INTERRUPT) {
        if (HAL_UARTEx_ReceiveToIdle_IT(huart, uartHandle.rxBuffer, uartHandle.rxSize) != HAL_OK) {
            log_debug("Failed to restart interrupt ReceiveToIdle");
            HAL_UART_Receive_IT(huart, uartHandle.rxBuffer, uartHandle.rxSize);
        }
    }
}

UART_Status_t UART_Init(UART_Handle_t* handle, const UART_Config_t* config)
{
    log_debug("UART: Initializing UART");

    if (handle == NULL || config == NULL || config->instance == NULL) {
        log_debug("UART handle or config is NULL");
        return UART_ERROR;
    }

    /* Deinitialize previous configuration if initialized */
    if (handle->isInitialized) {
        UART_DeInit(handle);
    }

    /* Store configuration */
    handle->config = *config;

    /* Configure UART base settings */
    if (handle->huart == NULL) {
        log_debug("UART HAL handle is NULL");
        return UART_ERROR;
    }

    memset(handle->huart, 0, sizeof(UART_HandleTypeDef));

    /* Configure UART base settings */
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

    /* Initialize mode-specific functionality */
    UART_Status_t status = UART_ERROR;
    switch (config->mode) {
        case UART_MODE_DMA:
            status = UART_DMA_Init(handle);
            break;
        case UART_MODE_INTERRUPT:
            status = UART_IT_Init(handle);
            break;
        case UART_MODE_BLOCKING:
            status = UART_Blocking_Init(handle);
            break;
        default:
            log_debug("Invalid UART mode");
            return UART_ERROR;
    }

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

    if (HAL_UART_DeInit(handle->huart) != HAL_OK) {
        log_debug("UART deinitialization failed");
        return UART_ERROR;
    }

    /* Deinitialize MSP resources */
    HAL_UART_MspDeInit(handle->huart);

    /* Reset handle state */
    handle->isInitialized = false;
    handle->rxBuffer = NULL;
    handle->txBuffer = NULL;
    handle->rxSize = 0;
    memset(&handle->config, 0, sizeof(UART_Config_t));

    return UART_OK;
}

/* Helper function to handle UART mode-specific operations */
static UART_Status_t UART_HandleMode(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout, bool isTransmit)
{
    if (handle == NULL || data == NULL || size == 0) {
        log_debug("UART handle, data is NULL or size is 0");
        return UART_ERROR;
    }

    if (!handle->isInitialized) {
        log_debug("UART not initialized");
        return UART_ERROR;
    }

    log_debug("UART mode: %d", handle->config.mode);
    switch (handle->config.mode) {
        case UART_MODE_DMA:
            return isTransmit ? UART_DMA_Transmit(handle, data, size, timeout) : UART_DMA_Receive(handle, (uint8_t*)data, size, timeout);
        case UART_MODE_INTERRUPT:
            return isTransmit ? UART_IT_Transmit(handle, data, size, timeout) : UART_IT_Receive(handle, (uint8_t*)data, size, timeout);
        case UART_MODE_BLOCKING:
            return isTransmit ? UART_Blocking_Transmit(handle, data, size, timeout) : UART_Blocking_Receive(handle, (uint8_t*)data, size, timeout);
        default:
            return UART_ERROR;
    }
}

UART_Status_t UART_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    return UART_HandleMode(handle, data, size, timeout, true);
}

UART_Status_t UART_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    return UART_HandleMode(handle, data, size, timeout, false);
}


/**
 * @brief HAL UART Rx Event callback (for IDLE detection)
 * @param huart UART handle pointer
 * @param Size Number of bytes received
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart != uartHandle.huart || Size == 0) {
        return;
    }

    if (uartHandle.config.mode == UART_MODE_DMA ||
        uartHandle.config.mode == UART_MODE_INTERRUPT) {
        log_debug("UART Rx Event (IDLE) - Size: %d", Size);
        UART_RingBuffer_PutData(uartHandle.rxBuffer, Size);
        rxComplete = 1;
        UART_RestartReceive(huart);
    }
}

/**
 * @brief HAL UART transmit complete callback
 * @param huart UART handle pointer
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != uartHandle.huart) {
        return;  // Not our UART
    }

    /* Set transmission complete flag */
    txComplete = 1;
    log_debug("UART TX Complete");
}

/**
 * @brief HAL UART receive complete callback
 * @param huart UART handle pointer
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != uartHandle.huart) {
        return;  // Not our UART
    }

    if (uartHandle.config.mode == UART_MODE_INTERRUPT) {
        log_debug("IT Received data complete");

        /* HAL_UART_Receive_IT delivers a single byte; ReceiveToIdle reports via RxEvent. */
        if (huart->RxXferSize == 1) {
            UART_RingBuffer_PutData(uartHandle.rxBuffer, 1);
        }
        rxComplete = 1;
        UART_RestartReceive(huart);
    } else if (uartHandle.config.mode == UART_MODE_DMA) {
        log_debug("DMA Full Buffer Received: %d bytes", uartHandle.rxSize);

        UART_RingBuffer_PutData(uartHandle.rxBuffer, uartHandle.rxSize);
        rxComplete = 1;
        UART_RestartReceive(huart);
    }
}

/**
 * @brief HAL UART error callback
 * @param huart UART handle pointer
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart != uartHandle.huart) {
        return;  // Not our UART
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

    /* Reset the UART peripheral first to clear stuck errors */
    HAL_UART_AbortReceive(huart);
    huart->ErrorCode = HAL_UART_ERROR_NONE;

    if (uartHandle.config.mode == UART_MODE_INTERRUPT) {
        HAL_UART_Init(huart);
    }

    UART_RestartReceive(huart);
}

/**
  * @brief UART MSP Initialization
  * This function configures the hardware resources used in this example
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    static DMA_HandleTypeDef hdma_usart1_tx;
    static DMA_HandleTypeDef hdma_usart1_rx;

    if (huart->Instance == USART1) {

        __HAL_RCC_USART1_CLK_ENABLE();

        /* USART1 GPIO Configuration: PA9 -> USART1_TX, PA10 -> USART1_RX */
        /* GPIO driver enables the GPIOA port clock */
        GPIO_InitTypeDef GPIO_InitStruct = {0};
        GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
        GPIO_Driver_Pin_Init(GPIOA, &GPIO_InitStruct);

        if (uartHandle.config.mode == UART_MODE_DMA) {
            __HAL_RCC_DMA2_CLK_ENABLE();
            /* DMA configuration for USART1_TX */
            hdma_usart1_tx.Instance = DMA2_Stream7;
            hdma_usart1_tx.Init.Channel = DMA_CHANNEL_4;
            hdma_usart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
            hdma_usart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
            hdma_usart1_tx.Init.MemInc = DMA_MINC_ENABLE;
            hdma_usart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            hdma_usart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
            hdma_usart1_tx.Init.Mode = DMA_NORMAL;
            hdma_usart1_tx.Init.Priority = DMA_PRIORITY_LOW;
            hdma_usart1_tx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

            if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK) {
                Error_Handler();
            }
            __HAL_LINKDMA(huart, hdmatx, hdma_usart1_tx);

            /* DMA configuration for USART1_RX */
            hdma_usart1_rx.Instance = DMA2_Stream5;
            hdma_usart1_rx.Init.Channel = DMA_CHANNEL_4;
            hdma_usart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
            hdma_usart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
            hdma_usart1_rx.Init.MemInc = DMA_MINC_ENABLE;
            hdma_usart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
            hdma_usart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
            hdma_usart1_rx.Init.Mode = DMA_NORMAL;
            hdma_usart1_rx.Init.Priority = DMA_PRIORITY_HIGH;
            hdma_usart1_rx.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

            if (HAL_DMA_Init(&hdma_usart1_rx) != HAL_OK) {
                Error_Handler();
            }
            __HAL_LINKDMA(huart, hdmarx, hdma_usart1_rx);

            /* NVIC configuration for DMA interrupts */
            HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 2, 0);
            HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
            HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 2, 0);
            HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
        }
        /* Always enable USART1 IRQ for both DMA and interrupt modes */
        if (uartHandle.config.mode == UART_MODE_DMA || uartHandle.config.mode == UART_MODE_INTERRUPT) {
            HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
            HAL_NVIC_EnableIRQ(USART1_IRQn);
        }
    }
}

/**
  * @brief UART MSP De-Initialization
  * This function frees the hardware resources used in this example
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if (huart->Instance == USART1) {
        /* Disable UART peripheral clock */
        __HAL_RCC_USART1_CLK_DISABLE();

        /* Deinitialize GPIO pins */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);

        /* Disable DMA if used */
        if (uartHandle.config.mode == UART_MODE_DMA) {
            if (huart->hdmatx != NULL) {
                HAL_DMA_DeInit(huart->hdmatx);
            }
            if (huart->hdmarx != NULL) {
                HAL_DMA_DeInit(huart->hdmarx);
            }
            HAL_NVIC_DisableIRQ(DMA2_Stream7_IRQn);
            HAL_NVIC_DisableIRQ(DMA2_Stream5_IRQn);
        }

        /* Always disable UART interrupt */
        HAL_NVIC_DisableIRQ(USART1_IRQn);
    }
}
