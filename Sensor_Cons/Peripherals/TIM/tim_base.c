/**
 * @file tim_base.c
 * @brief Timer base mode driver implementation (counting, delays, time-base generation)
 *
 * This module implements basic timer counting functionality on STM32F4 timers.
 * It wraps the HAL base timer functions with a simpler API.
 *
 * Key Implementation Details:
 * - Uses up-counting mode (CNT counts 0..ARR, then overflows to 0)
 * - Auto-reload preload disabled (ARR changes take effect immediately)
 * - Clock division = DIV1 (no additional divider)
 * - All functions validate htim != NULL before calling HAL.
 * - Logging via log.h for initialization status.
 */

#include "tim_base.h"
#include "log.h"

/* ========================== Base Timer ========================== */

HAL_StatusTypeDef TIM_Init(TIM_HandleTypeDef *htim,
                           TIM_TypeDef *instance,
                           uint32_t prescaler,
                           uint32_t period)
{
    log_debug("TIM: Initializing Timer");

    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }

    htim->Instance = instance;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    HAL_StatusTypeDef status = HAL_TIM_Base_Init(htim);
    if (status == HAL_OK) {
        log_debug("TIM: Timer initialized successfully");
    }
    return status;
}

HAL_StatusTypeDef TIM_Start(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Start(htim);
}

HAL_StatusTypeDef TIM_Stop(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Stop(htim);
}

HAL_StatusTypeDef TIM_Start_IT(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Start_IT(htim);
}

HAL_StatusTypeDef TIM_Stop_IT(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Base_Stop_IT(htim);
}

uint32_t TIM_GetCounter(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return 0U;
    }
    return __HAL_TIM_GET_COUNTER(htim);
}

void TIM_SetCounter(TIM_HandleTypeDef *htim, uint32_t value)
{
    if (htim != NULL) {
        __HAL_TIM_SET_COUNTER(htim, value);
    }
}
