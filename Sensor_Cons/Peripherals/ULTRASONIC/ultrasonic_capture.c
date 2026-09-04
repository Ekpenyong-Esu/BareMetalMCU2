/**
 ******************************************************************************
 * @file    ultrasonic_capture.c
 * @brief   Capture timer setup and microsecond timing (internal)
 ******************************************************************************
 */

#include "ultrasonic_capture.h"
#include "ultrasonic_convert.h"
#include "tim_base.h"
#include "tim_clock.h"
#include "tim_ic.h"
#include "log.h"

/** Largest (prescaler + 1) a 16-bit timer prescaler can express */
#define ULTRASONIC_MAX_TIMER_DIVIDER 0x10000U

ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_Init(TIM_HandleTypeDef *htim, uint32_t channel) {
    uint32_t timerClock = 0;
    uint32_t divider = 0;

    if (htim == NULL || htim->Instance == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    if (!TIM_Clock_Enable(htim->Instance)) {
        return ULTRASONIC_INVALID_PARAM;
    }

    timerClock = TIM_Clock_GetHz(htim->Instance);
    divider = timerClock / ULTRASONIC_ECHO_TICK_HZ;
    if (divider == 0U || divider > ULTRASONIC_MAX_TIMER_DIVIDER ||
        (timerClock % ULTRASONIC_ECHO_TICK_HZ) != 0U) {
        log_error("ULTRASONIC: timer clock %lu Hz cannot be divided to 1 MHz",
                  (unsigned long)timerClock);
        return ULTRASONIC_INVALID_PARAM;
    }

    if (TIM_IC_Init(htim, htim->Instance, divider - 1U, ULTRASONIC_ECHO_PERIOD) != HAL_OK) {
        return ULTRASONIC_ERROR;
    }

    if (TIM_IC_ConfigChannel(htim, channel, TIM_ICPOLARITY_RISING) != HAL_OK) {
        return ULTRASONIC_ERROR;
    }

    if (TIM_Start(htim) != HAL_OK) {
        return ULTRASONIC_ERROR;
    }

    return ULTRASONIC_OK;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_DeInit(TIM_HandleTypeDef *htim, uint32_t channel) {
    if (htim == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    (void)TIM_IC_Stop(htim, channel);

    return (TIM_Stop(htim) == HAL_OK) ? ULTRASONIC_OK : ULTRASONIC_ERROR;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_Arm(TIM_HandleTypeDef *htim, uint32_t channel) {
    if (htim == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    if (TIM_IC_ConfigChannel(htim, channel, TIM_ICPOLARITY_RISING) != HAL_OK) {
        return ULTRASONIC_ERROR;
    }

    return (TIM_IC_Start_IT(htim, channel) == HAL_OK) ? ULTRASONIC_OK : ULTRASONIC_ERROR;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_Disarm(TIM_HandleTypeDef *htim, uint32_t channel) {
    if (htim == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    (void)TIM_IC_Stop(htim, channel);

    /* Leave the channel on the rising edge so the next trigger starts clean. */
    return (TIM_IC_ConfigChannel(htim, channel, TIM_ICPOLARITY_RISING) == HAL_OK)
               ? ULTRASONIC_OK
               : ULTRASONIC_ERROR;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_CAPTURE_ExpectFalling(TIM_HandleTypeDef *htim,
                                                          uint32_t channel) {
    if (htim == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    return (TIM_IC_ConfigChannel(htim, channel, TIM_ICPOLARITY_FALLING) == HAL_OK)
               ? ULTRASONIC_OK
               : ULTRASONIC_ERROR;
}

void ULTRASONIC_CAPTURE_DelayMicroseconds(TIM_HandleTypeDef *htim, uint32_t microseconds) {
    uint32_t start = 0;

    if (htim == NULL || microseconds == 0U) {
        return;
    }

    start = TIM_GetCounter(htim);

    /* A stopped timer would never advance the counter, so bound the spin
       instead of trusting it; the pulse is only a few microseconds. */
    uint32_t guard = microseconds * ULTRASONIC_DELAY_GUARD_LOOPS;

    while (guard-- > 0U) {
        uint32_t width = ULTRASONIC_EchoWidth(start, TIM_GetCounter(htim), ULTRASONIC_ECHO_PERIOD);
        if (width >= microseconds) {
            return;
        }
        /* 1 tick == 1 us */
    }
}
