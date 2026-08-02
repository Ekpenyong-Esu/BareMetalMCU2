/**
 * @file tim_encoder.c
 * @brief Timer quadrature encoder interface driver (rotary encoder reading)
 */

#include "tim_encoder.h"

/* ========================== Encoder Interface ========================== */

HAL_StatusTypeDef TIM_Encoder_Init(TIM_HandleTypeDef *htim,
                                   TIM_TypeDef *instance,
                                   uint32_t period)
{
    if (htim == NULL || instance == NULL) {
        return HAL_ERROR;
    }

    htim->Instance = instance;
    htim->Init.Prescaler = 0U;
    htim->Init.CounterMode = TIM_COUNTERMODE_UP;
    htim->Init.Period = period;
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    /* Encoder configuration: count on both TI1 and TI2 edges */
    TIM_Encoder_InitTypeDef sEncoderConfig = {0};
    sEncoderConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sEncoderConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC1Filter = 0U;
    sEncoderConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sEncoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sEncoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;
    sEncoderConfig.IC2Filter = 0U;

    return HAL_TIM_Encoder_Init(htim, &sEncoderConfig);
}

HAL_StatusTypeDef TIM_Encoder_Start(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Encoder_Start(htim, TIM_CHANNEL_ALL);
}

HAL_StatusTypeDef TIM_Encoder_Stop(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_Encoder_Stop(htim, TIM_CHANNEL_ALL);
}

int32_t TIM_Encoder_GetCount(TIM_HandleTypeDef *htim)
{
    if (htim == NULL) {
        return 0;
    }
    return (int32_t)__HAL_TIM_GET_COUNTER(htim);
}

void TIM_Encoder_Reset(TIM_HandleTypeDef *htim)
{
    if (htim != NULL) {
        __HAL_TIM_SET_COUNTER(htim, 0U);
    }
}
