/**
 * @file waveform_timer.c
 * @brief TIM7 pacing for the waveform app
 */

#include "waveform_timer.h"

#include "log.h"
#include "stm32f4xx_hal.h"
#include "tim.h"

/* TIM7 runs from APB1 (84 MHz) on the STM32F429I-DISC1, so PSC=83 divides to
 * 1 MHz and ARR=2000 produces a 2 ms update event. */
#define WAVEFORM_TIM_PRESCALER  83U
#define WAVEFORM_TIM_PERIOD     2000U

/* Numerically >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5), otherwise
 * the tick handler may not call FreeRTOS ...FromISR APIs. */
#define WAVEFORM_TIM_IRQ_PRIORITY  6U

static TIM_HandleTypeDef    s_tim;
static Waveform_TimerTickFn s_onTick;

bool Waveform_TimerInit(void)
{
    if (TIM_Init(&s_tim, TIM7, WAVEFORM_TIM_PRESCALER, WAVEFORM_TIM_PERIOD) != HAL_OK) {
        log_error("DAC waveform: TIM7 base init failed");
        return false;
    }

    TIM_MasterConfigTypeDef masterConfig = {0};
    masterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    masterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(&s_tim, &masterConfig) != HAL_OK) {
        log_error("DAC waveform: TIM7 master config failed");
        return false;
    }
    return true;
}

void Waveform_TimerOnTick(Waveform_TimerTickFn onTick)
{
    s_onTick = onTick;
}

bool Waveform_TimerStart(void)
{
    HAL_NVIC_SetPriority(TIM7_IRQn, WAVEFORM_TIM_IRQ_PRIORITY, 0U);
    HAL_NVIC_EnableIRQ(TIM7_IRQn);

    if (TIM_Start_IT(&s_tim) != HAL_OK) {
        log_error("DAC waveform: TIM7 start failed");
        return false;
    }
    return true;
}

void Waveform_TimerIrqHandler(void)
{
    /* The flag is cleared here rather than via HAL_TIM_IRQHandler because the
     * timebase already claims HAL_TIM_PeriodElapsedCallback. */
    if (__HAL_TIM_GET_FLAG(&s_tim, TIM_FLAG_UPDATE) == 0U ||
        __HAL_TIM_GET_IT_SOURCE(&s_tim, TIM_IT_UPDATE) == 0U) {
        return;
    }

    __HAL_TIM_CLEAR_IT(&s_tim, TIM_IT_UPDATE);

    if (s_onTick != NULL) {
        s_onTick();
    }
}
