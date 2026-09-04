/**
 * @file tim_ic.c
 * @brief Timer input capture driver (frequency/pulse measurement)
 */

#include "tim_ic.h"

/* ========================== Input Capture ========================== */

HAL_StatusTypeDef TIM_IC_Init(TIM_HandleTypeDef *htim, TIM_TypeDef *instance, uint32_t prescaler,
                              uint32_t period) {
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }

    htim->Instance = instance;
    htim->Init.Prescaler = prescaler;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    return HAL_TIM_IC_Init(htim);
}

HAL_StatusTypeDef TIM_IC_ConfigChannel(TIM_HandleTypeDef *htim, uint32_t channel,
                                       uint32_t polarity) {
    if (htim == NULL) {
        return HAL_ERROR;
    }

    TIM_IC_InitTypeDef sConfigIC = {0};
    sConfigIC.ICPolarity = polarity;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0U;

    return HAL_TIM_IC_ConfigChannel(htim, &sConfigIC, channel);
}

HAL_StatusTypeDef TIM_IC_Start(TIM_HandleTypeDef *htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Start(htim, channel);
}

HAL_StatusTypeDef TIM_IC_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Start_IT(htim, channel);
}

HAL_StatusTypeDef TIM_IC_Stop(TIM_HandleTypeDef *htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Stop(htim, channel);
}

HAL_StatusTypeDef TIM_IC_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel) {
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_IC_Stop_IT(htim, channel);
}

uint32_t TIM_IC_GetCapture(TIM_HandleTypeDef *htim, uint32_t channel) {
    if (htim == NULL) {
        return 0U;
    }
    return HAL_TIM_ReadCapturedValue(htim, channel);
}
