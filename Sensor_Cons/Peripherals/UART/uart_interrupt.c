/**
 * @file uart_interrupt.c
 * @brief UART Interrupt mode implementation for STM32F429I-DISC1
 */

#include "uart_interrupt.h"
#include "uart_config.h"
#include "log.h"

/* External reference to global UART handle */
extern UART_Handle_t uartHandle;

UART_Status_t UART_IT_Init(UART_Handle_t* handle)
{
    if (handle == NULL || handle->huart == NULL) {
        log_debug("UART handle or huart is NULL");
        return UART_ERROR;
    }

    /* Initialize ring buffer for continuous reception */
    UART_RingBuffer_Init();

    /* Enable UART IDLE line detection - helps with command detection */
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR);


    handle->isInitialized = true;
    return UART_OK;
}

UART_Status_t UART_IT_Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        log_debug("UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    txComplete = 0;
    HAL_StatusTypeDef status = HAL_UART_Transmit_IT(handle->huart, (uint8_t*)data, size);
    if (status != HAL_OK) {
        log_debug("UART Transmit failed: %d", status);
        return UART_ERROR;
    }

    if (timeout > 0) {
        uint32_t tickstart = HAL_GetTick();
        while (!txComplete) {
            if ((HAL_GetTick() - tickstart) > timeout) {
                log_debug("UART Transmit timeout");
                return UART_TIMEOUT_ERROR;
            }
        }
    }

    return UART_OK;
}

UART_Status_t UART_IT_Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    if (handle == NULL || handle->huart == NULL || data == NULL || size == 0) {
        log_debug("UART handle, huart, data is NULL or size is 0");
        return UART_ERROR;
    }

    rxComplete = 0;
    /* Use HAL_UARTEx_ReceiveToIdle_IT for better command reception with IDLE detection */
    HAL_StatusTypeDef status = HAL_UARTEx_ReceiveToIdle_IT(handle->huart, data, size);
    if (status != HAL_OK) {
        log_debug("UART ReceiveToIdle failed: %d", status);
        /* Fall back to regular interrupt reception if ReceiveToIdle fails */
        status = HAL_UART_Receive_IT(handle->huart, data, size);
        if (status != HAL_OK) {
            log_debug("Regular UART Receive_IT also failed: %d", status);
            return UART_ERROR;
        }
    }


    if (timeout > 0) {
        uint32_t tickstart = HAL_GetTick();
        while (!rxComplete) {
            if ((HAL_GetTick() - tickstart) > timeout) {
                log_debug("UART Receive timeout");
                return UART_TIMEOUT_ERROR;
            }
        }
    }

    return UART_OK;
}

/*
 * UART interrupt vector is owned by the interrupt layer, not this driver.
 * USART1_IRQHandler() is defined in Core/Src/stm32f4xx_it.c and dispatches
 * here via HAL_UART_IRQHandler(uartHandle.huart).
 */
