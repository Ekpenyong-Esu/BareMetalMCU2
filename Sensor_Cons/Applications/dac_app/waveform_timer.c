/**
 * @file waveform_timer.c
 * @brief The DAC's trigger source: TIM7 raising TRGO every 2 ms
 */

#include "waveform_timer.h"

#include "log.h"
#include "stm32f4xx_hal.h"
#include "tim.h"

/* TIM7 is clocked from APB1 at 84 MHz, so PSC=83 divides to 1 MHz and
 * ARR=2000 gives one update event every 2 ms. */
#define WAVEFORM_TIM_PRESCALER  83U
#define WAVEFORM_TIM_PERIOD     2000U

/* Numerically >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5), otherwise
 * the tick handler may not call FreeRTOS ...FromISR APIs. */
#define WAVEFORM_TIM_IRQ_PRIORITY  6U

/* Private because nothing outside this file has any use for the handle. */
static TIM_HandleTypeDef    s_tim;
static Waveform_TimerTickFn s_onTick;

bool Waveform_TimerStart(Waveform_TimerTickFn onTick)
{
    s_onTick = onTick;

    /* TIM_Init does not do this, and no MspInit covers TIM7. */
    if (!TIM_Clock_Enable(TIM7)) {
        log_error("DAC waveform: TIM7 clock enable failed");
        return false;
    }

    if (TIM_Init(&s_tim, TIM7, WAVEFORM_TIM_PRESCALER, WAVEFORM_TIM_PERIOD) != HAL_OK) {
        log_error("DAC waveform: TIM7 init failed");
        return false;
    }

    TIM_MasterConfigTypeDef masterConfig = {0};
    masterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    masterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(&s_tim, &masterConfig) != HAL_OK) {
        log_error("DAC waveform: TIM7 TRGO routing failed");
        return false;
    }

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
    /* The flag is cleared here rather than through HAL_TIM_IRQHandler because
     * the HAL timebase already claims HAL_TIM_PeriodElapsedCallback. */
    if (__HAL_TIM_GET_FLAG(&s_tim, TIM_FLAG_UPDATE) == 0U ||
        __HAL_TIM_GET_IT_SOURCE(&s_tim, TIM_IT_UPDATE) == 0U) {
        return;
    }

    __HAL_TIM_CLEAR_IT(&s_tim, TIM_IT_UPDATE);

    if (s_onTick != NULL) {
        s_onTick();
    }
}
