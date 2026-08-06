/**
 * @file uart_interrupt.c
 * @brief Interrupt-driven transfer mode
 */

#include "uart_interrupt.h"
#include "uart_core.h"
#include "log.h"

/* Reception is armed the same way whether it is the first call or a re-arm
   from interrupt context, so both paths share this. */
static bool StartReceive(UART_Handle_t* handle, uint8_t* data, uint16_t size)
{
    if (HAL_UARTEx_ReceiveToIdle_IT(handle->huart, data, size) == HAL_OK) {
        return true;
    }

    log_debug("UART ReceiveToIdle failed, falling back to fixed-length receive");
    return HAL_UART_Receive_IT(handle->huart, data, size) == HAL_OK;
}

static UART_Status_t Init(UART_Handle_t* handle)
{
    /* IDLE marks the end of a frame; ERR surfaces overrun and framing faults. */
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_IDLE);
    __HAL_UART_ENABLE_IT(handle->huart, UART_IT_ERR);

    return UART_OK;
}

static UART_Status_t Transmit(UART_Handle_t* handle, const uint8_t* data, uint16_t size, uint32_t timeout)
{
    handle->txComplete = false;

    if (HAL_UART_Transmit_IT(handle->huart, (uint8_t*)data, size) != HAL_OK) {
        log_debug("UART Transmit failed");
        return UART_ERROR;
    }

    if (timeout == 0) {
        return UART_OK;
    }

    return UART_WaitForFlag(&handle->txComplete, timeout);
}

static UART_Status_t Receive(UART_Handle_t* handle, uint8_t* data, uint16_t size, uint32_t timeout)
{
    handle->rxComplete = false;

    if (!StartReceive(handle, data, size)) {
        log_debug("UART Receive failed to start");
        return UART_ERROR;
    }

    if (timeout == 0) {
        return UART_OK;
    }

    return UART_WaitForFlag(&handle->rxComplete, timeout);
}

static void RearmReceive(UART_Handle_t* handle)
{
    StartReceive(handle, handle->rxBuffer, handle->rxSize);
}

static uint16_t CompletedReceiveSize(const UART_Handle_t* handle, const UART_HandleTypeDef* huart)
{
    (void)handle;

    /* ReceiveToIdle reports its length through HAL_UARTEx_RxEventCallback, so
       the only bytes still owed here come from a fixed single-byte receive. */
    return (huart->RxXferSize == 1) ? 1 : 0;
}

static void RecoverFromError(UART_Handle_t* handle)
{
    /* Interrupt mode needs the peripheral rebuilt before reception can restart. */
    HAL_UART_Init(handle->huart);
}

const UART_ModeOps_t UART_InterruptOps = {
    .name = "interrupt",
    .init = Init,
    .transmit = Transmit,
    .receive = Receive,
    .rearmReceive = RearmReceive,
    .completedReceiveSize = CompletedReceiveSize,
    .recoverFromError = RecoverFromError,
};

/*
 * The UART interrupt vector is owned by Core, not this driver.
 * USART1_IRQHandler() is defined in Core/Src/stm32f4xx_it.c and dispatches
 * into the callbacks in uart_events.c via HAL_UART_IRQHandler().
 */
