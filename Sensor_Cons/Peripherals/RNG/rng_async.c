/**
 * @file rng_async.c
 * @brief Interrupt-driven random number generation
 */

#include "rng_async.h"
#include "rng_core.h"

static RNG_DataCallback_t s_callback = NULL;
static volatile bool s_pending = false;
static bool s_interruptEnabled = false;

RNG_StatusTypeDef RNG_EnableInterrupt(void)
{
    if (!RNG_IsInitialized()) {
        return RNG_NOT_READY;
    }

    HAL_NVIC_SetPriority(HASH_RNG_IRQn, RNG_IRQ_PRIORITY, RNG_IRQ_SUBPRIORITY);
    HAL_NVIC_EnableIRQ(HASH_RNG_IRQn);
    s_interruptEnabled = true;

    return RNG_OK;
}

RNG_StatusTypeDef RNG_DisableInterrupt(void)
{
    RNG_HandleTypeDef *hrng = RNG_GetHandle();

    if (hrng != NULL) {
        __HAL_RNG_DISABLE_IT(hrng);
    }

    HAL_NVIC_DisableIRQ(HASH_RNG_IRQn);
    s_interruptEnabled = false;
    s_pending = false;
    s_callback = NULL;

    return RNG_OK;
}

RNG_StatusTypeDef RNG_GenerateAsync(RNG_DataCallback_t callback)
{
    RNG_HandleTypeDef *hrng = RNG_GetHandle();

    if (callback == NULL) {
        return RNG_ERROR;
    }

    if (hrng == NULL) {
        return RNG_NOT_READY;
    }

    /* Without an enabled vector the completion would never be serviced. */
    if (!s_interruptEnabled) {
        return RNG_NOT_READY;
    }

    if (s_pending) {
        return RNG_BUSY;
    }

    s_callback = callback;
    s_pending = true;

    if (HAL_RNG_GenerateRandomNumber_IT(hrng) != HAL_OK) {
        s_pending = false;
        s_callback = NULL;
        return RNG_ERROR;
    }

    return RNG_OK;
}

bool RNG_IsGenerationPending(void)
{
    return s_pending;
}

void RNG_IRQHandler(void)
{
    RNG_HandleTypeDef *hrng = RNG_GetHandle();

    if (hrng != NULL) {
        HAL_RNG_IRQHandler(hrng);
    }
}

void HAL_RNG_ReadyDataCallback(RNG_HandleTypeDef *hrng, uint32_t random32bit)
{
    RNG_DataCallback_t callback = s_callback;

    (void)hrng;

    s_pending = false;
    s_callback = NULL;

    if (callback != NULL) {
        callback(random32bit);
    }
}

void HAL_RNG_ErrorCallback(RNG_HandleTypeDef *hrng)
{
    (void)hrng;

    /* Drop the request rather than leaving the caller waiting forever. */
    s_pending = false;
    s_callback = NULL;
}
