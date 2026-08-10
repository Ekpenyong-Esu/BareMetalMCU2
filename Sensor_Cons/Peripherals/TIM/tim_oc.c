/**
 * @file tim_oc.c
 * @brief Timer output compare driver (timing events)
 */

#include "tim_oc.h"

/* ========================== Output Compare ========================== */

HAL_StatusTypeDef TIM_OC_Init(TIM_HandleTypeDef *htim,
                              TIM_TypeDef *instance,
                              uint32_t prescaler,
                              uint32_t period)
{
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }

    htim->Instance = instance;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; // internal filter clock and div1 means means no division
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    return HAL_TIM_OC_Init(htim);
}

HAL_StatusTypeDef TIM_OC_ConfigChannel(TIM_HandleTypeDef *htim,
                                       uint32_t channel,
                                       uint32_t pulse)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
    sConfigOC.Pulse = pulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    return HAL_TIM_OC_ConfigChannel(htim, &sConfigOC, channel);
}

HAL_StatusTypeDef TIM_OC_Start(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_OC_Start(htim, channel);
}

HAL_StatusTypeDef TIM_OC_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_OC_Start_IT(htim, channel);
}

HAL_StatusTypeDef TIM_OC_Stop(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_OC_Stop(htim, channel);
}

HAL_StatusTypeDef TIM_OC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_OC_Stop_IT(htim, channel);
}
