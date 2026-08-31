/**
 * @file tim_pwm.c
 * @brief Timer PWM output driver (LED dimming, motor control)
 */

#include "tim_pwm.h"

#include "tim_clock.h"

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

HAL_StatusTypeDef TIM_PWM_InitHz(TIM_HandleTypeDef *htim,
                                 TIM_TypeDef *instance,
                                 uint32_t frequencyHz,
                                 uint32_t steps)
{
    if (htim == NULL || frequencyHz == 0u || steps == 0u ||
        !TIM_Clock_HasOutputChannels(instance) || !TIM_Clock_Enable(instance)) {
        return HAL_ERROR;
    }

    uint32_t divider = TIM_Clock_GetHz(instance) / (frequencyHz * steps);

    if (divider == 0u || divider > 0x10000u) {
        return HAL_ERROR;
    }

    return TIM_PWM_Init(htim, instance, divider - 1u, steps - 1u);
}

HAL_StatusTypeDef TIM_PWM_ConfigChannel(TIM_HandleTypeDef *htim,
                                        uint32_t channel,
                                        uint32_t pulse,
                                        uint32_t polarity)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }

    TIM_OC_InitTypeDef sConfigOC = {0};
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = pulse;
    sConfigOC.OCPolarity = polarity;
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

HAL_StatusTypeDef TIM_PWM_Start_IT(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Start_IT(htim, channel);
}

HAL_StatusTypeDef TIM_PWM_Stop(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Stop(htim, channel);
}

HAL_StatusTypeDef TIM_PWM_Stop_IT(TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (htim == NULL) {
        return HAL_ERROR;
    }
    return HAL_TIM_PWM_Stop_IT(htim, channel);
}

void TIM_PWM_SetDuty(TIM_HandleTypeDef *htim, uint32_t channel, uint32_t pulse)
{
    if (htim == NULL) {
        return;
    }
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}
