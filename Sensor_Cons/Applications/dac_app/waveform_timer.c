/**
 * @file waveform_timer.c
 * @brief TIM6 pacing for the waveform app
 */

#include "waveform_timer.h"

#include "log.h"
#include "tim.h"

/* TIM6 runs at 1 MHz on the STM32F429I-DISC1 (APB1 = 84 MHz), so PSC=83
 * divides to 1 MHz and ARR=2000 produces a 2 ms update event. */
#define WAVEFORM_TIM_PRESCALER  83U
#define WAVEFORM_TIM_PERIOD     2000U

bool Waveform_TimerInit(TIM_HandleTypeDef *tim)
{
    if (TIM_Init(tim, TIM6, WAVEFORM_TIM_PRESCALER, WAVEFORM_TIM_PERIOD) != HAL_OK) {
        log_error("DAC waveform: TIM6 base init failed");
        return false;
    }
    return true;
}

bool Waveform_TimerRouteTrigger(TIM_HandleTypeDef *tim)
{
    TIM_MasterConfigTypeDef masterConfig = {0};
    masterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    masterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(tim, &masterConfig) != HAL_OK) {
        log_error("DAC waveform: TIM6 master config failed");
        return false;
    }
    return true;
}

bool Waveform_TimerTickArrived(TIM_HandleTypeDef *tim)
{
    return __HAL_TIM_GET_FLAG(tim, TIM_FLAG_UPDATE) != 0U;
}

void Waveform_TimerAckTick(TIM_HandleTypeDef *tim)
{
    __HAL_TIM_CLEAR_FLAG(tim, TIM_FLAG_UPDATE);
}
