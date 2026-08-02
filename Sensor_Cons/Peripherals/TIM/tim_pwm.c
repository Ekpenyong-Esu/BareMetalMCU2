/**
 * @file tim_pwm.c
 * @brief Timer PWM output driver (LED dimming, motor control)
 */

#include "tim_pwm.h"

/* ========================== PWM Output ========================== */

HAL_StatusTypeDef TIM_PWM_Init(TIM_HandleTypeDef *htim,
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
    htim->Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim->Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    return HAL_TIM_PWM_Init(htim);
}

HAL_StatusTypeDef TIM_PWM_ConfigChannel(TIM_HandleTypeDef *htim,
                                        uint32_t channel,
                                        uint32_t pulse)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = pulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    return HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, channel);
}

HAL_StatusTypeDef TIM_PWM_Start(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Start(htim, channel);
}

HAL_StatusTypeDef TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Stop(htim, channel);
}

void TIM_PWM_SetDuty(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse)
{
    if (htim == NULL) {
        return;
    }
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}
