/**
 ******************************************************************************
 * @file    stepper_timing.c
 * @brief   Microsecond time base backed by the caller's timer
 ******************************************************************************
 */

#include "stepper_timing.h"
#include "tim_base.h"
#include "tim_clock.h"
#include "log.h"

#define STEPPER_TICKS_PER_SECOND 1000000U

/* 16 bits are available on every timer this driver may be handed. */
#define STEPPER_COUNTER_PERIOD 0xFFFFU
#define STEPPER_COUNTER_MASK 0xFFFFU

/** Longest single wait; longer requests are split into repeats of this. */
#define STEPPER_MAX_WAIT_TICKS 0x8000U

STEPPER_StatusTypeDef STEPPER_TIMING_Init(TIM_HandleTypeDef *htim) {
    uint32_t timerHz = 0;
    uint32_t prescaler = 0;

    if (htim == NULL || htim->Instance == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    timerHz = TIM_Clock_GetHz(htim->Instance);
    if (timerHz < STEPPER_TICKS_PER_SECOND) {
        log_error("STEPPER: timer clock too slow for a 1 MHz time base");
        return STEPPER_ERROR;
    }

    prescaler = (timerHz / STEPPER_TICKS_PER_SECOND) - 1U;

    if (TIM_Init(htim, htim->Instance, prescaler, STEPPER_COUNTER_PERIOD) != HAL_OK) {
        return STEPPER_ERROR;
    }

    if (TIM_Start(htim) != HAL_OK) {
        return STEPPER_ERROR;
    }

    return STEPPER_OK;
}

void STEPPER_TIMING_DeInit(TIM_HandleTypeDef *htim) {
    if (htim != NULL && htim->Instance != NULL) {
        (void)TIM_Stop(htim);
    }
}

void STEPPER_TIMING_DelayMicroseconds(TIM_HandleTypeDef *htim, uint32_t microseconds) {
    uint32_t remaining = microseconds;

    while (remaining > 0U) {
        uint32_t slice = (remaining > STEPPER_MAX_WAIT_TICKS) ? STEPPER_MAX_WAIT_TICKS : remaining;
        uint32_t start = TIM_GetCounter(htim) & STEPPER_COUNTER_MASK;

        while (((TIM_GetCounter(htim) - start) & STEPPER_COUNTER_MASK) < slice) {
            /* Busy wait: a step interval is at most 100 ms and the caller is blocking anyway. */
        }

        remaining -= slice;
    }
}
