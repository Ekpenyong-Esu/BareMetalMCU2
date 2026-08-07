/**
 * @file eth_irq.c
 * @brief Ethernet interrupt control and event callbacks
 */

#include "eth_irq.h"
#include "eth_core.h"

static ETH_Handle_t *s_handle = NULL;
static bool s_interruptMode = false;

HAL_StatusTypeDef ETH_EnableInterrupts(ETH_Handle_t *handle)
{
    bool wasRunning;

    if ((handle == NULL) || !ETH_IsInitialized(handle)) {
        return HAL_ERROR;
    }

    wasRunning = ETH_IsReady(handle);

    if (wasRunning && (HAL_ETH_Stop(&handle->heth) != HAL_OK)) {
        return HAL_ERROR;
    }

    s_handle = handle;
    s_interruptMode = true;

    NVIC_ClearPendingIRQ(ETH_IRQn);
    HAL_NVIC_SetPriority(ETH_IRQn, ETH_IRQ_PRIORITY, ETH_IRQ_SUBPRIORITY);
    HAL_NVIC_EnableIRQ(ETH_IRQn);

    if (wasRunning) {
        return HAL_ETH_Start_IT(&handle->heth);
    }

    return HAL_OK;
}

HAL_StatusTypeDef ETH_DisableInterrupts(ETH_Handle_t *handle)
{
    bool wasRunning;

    if ((handle == NULL) || !ETH_IsInitialized(handle)) {
        return HAL_ERROR;
    }

    wasRunning = ETH_IsReady(handle);

    if (wasRunning && (HAL_ETH_Stop(&handle->heth) != HAL_OK)) {
        return HAL_ERROR;
    }

    HAL_NVIC_DisableIRQ(ETH_IRQn);
    s_interruptMode = false;
    s_handle = NULL;

    if (wasRunning) {
        return HAL_ETH_Start(&handle->heth);
    }

    return HAL_OK;
}

bool ETH_IsInterruptModeEnabled(void)
{
    return s_interruptMode;
}

void ETH_Driver_IRQHandler(void)
{
    if (s_handle != NULL) {
        HAL_ETH_IRQHandler(&s_handle->heth);
    }
}

/*
 * The HAL raises its own callbacks. Forward them so the driver level hooks
 * below are actually reached instead of remaining unreferenced.
 */
void HAL_ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
    ETH_TxCpltCallback(heth);
}

void HAL_ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    ETH_RxCpltCallback(heth);
}

void HAL_ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
    ETH_ErrorCallback(heth);
}

__weak void ETH_TxCpltCallback(ETH_HandleTypeDef *heth)
{
    (void)heth;
}

__weak void ETH_RxCpltCallback(ETH_HandleTypeDef *heth)
{
    (void)heth;
}

__weak void ETH_ErrorCallback(ETH_HandleTypeDef *heth)
{
    (void)heth;
}
