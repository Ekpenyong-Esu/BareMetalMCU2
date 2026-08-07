/**
  ******************************************************************************
  * @file    servo_pwm.c
  * @brief   Servo PWM transport - internal to the SERVO driver
  ******************************************************************************
  */

#include "servo_pwm.h"
#include "tim_pwm.h"
#include "tim_clock.h"
#include "gpio.h"
#include "log.h"

/* 50 Hz frame divided into 20000 compare steps gives exactly 1 us per step. */
#define SERVO_PWM_STEPS_PER_PERIOD   SERVO_PWM_PERIOD_US
#define SERVO_PWM_COUNTER_HZ         1000000U

static bool SERVO_PWM_ResolveAlternate(const TIM_TypeDef *instance, uint8_t *alternate)
{
    if (instance == TIM1 || instance == TIM2) {
        *alternate = GPIO_AF1_TIM1;
    } else if (instance == TIM3 || instance == TIM4 || instance == TIM5) {
        *alternate = GPIO_AF2_TIM3;
    } else if (instance == TIM8 || instance == TIM9 || instance == TIM10 || instance == TIM11) {
        *alternate = GPIO_AF3_TIM8;
    } else if (instance == TIM12 || instance == TIM13 || instance == TIM14) {
        *alternate = GPIO_AF9_TIM12;
    } else {
        /* Leaving Alternate at 0 would map the pin to SYS, not to the timer. */
        return false;
    }

    return true;
}

static bool SERVO_PWM_IsValidChannel(uint32_t channel)
{
    return (channel == TIM_CHANNEL_1 || channel == TIM_CHANNEL_2 ||
            channel == TIM_CHANNEL_3 || channel == TIM_CHANNEL_4);
}

SERVO_StatusTypeDef SERVO_PWM_Init(SERVO_Handle_t *hservo)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    uint8_t alternate;

    if (!SERVO_PWM_IsValidChannel(hservo->channel)) {
        log_error("SERVO: unsupported timer channel");
        return SERVO_INVALID_PARAM;
    }

    if (!SERVO_PWM_ResolveAlternate(hservo->htim->Instance, &alternate)) {
        log_error("SERVO: timer has no known GPIO alternate function");
        return SERVO_INVALID_PARAM;
    }

    if (TIM_Clock_GetHz(hservo->htim->Instance) % SERVO_PWM_COUNTER_HZ != 0U) {
        log_error("SERVO: timer clock cannot produce a 1 MHz counter");
        return SERVO_ERROR;
    }

    GPIO_InitStruct.Pin = hservo->gpioPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = alternate;
    GPIO_Driver_Pin_Init(hservo->gpioPort, &GPIO_InitStruct);

    if (TIM_PWM_InitHz(hservo->htim, hservo->htim->Instance,
                       SERVO_PWM_FREQUENCY_HZ, SERVO_PWM_STEPS_PER_PERIOD) != HAL_OK) {
        log_error("SERVO: PWM timer init failed");
        HAL_GPIO_DeInit(hservo->gpioPort, hservo->gpioPin);
        return SERVO_ERROR;
    }

    /* Without this the channel keeps its reset output-compare mode and never
       produces a PWM waveform, however the compare register is written. */
    if (TIM_PWM_ConfigChannel(hservo->htim, hservo->channel,
                              SERVO_DEFAULT_PULSE_WIDTH_US) != HAL_OK) {
        log_error("SERVO: PWM channel config failed");
        HAL_GPIO_DeInit(hservo->gpioPort, hservo->gpioPin);
        return SERVO_ERROR;
    }

    if (TIM_PWM_Start(hservo->htim, hservo->channel) != HAL_OK) {
        log_error("SERVO: PWM start failed");
        HAL_GPIO_DeInit(hservo->gpioPort, hservo->gpioPin);
        return SERVO_ERROR;
    }

    return SERVO_OK;
}

void SERVO_PWM_DeInit(SERVO_Handle_t *hservo)
{
    if (hservo->htim != NULL) {
        (void)TIM_PWM_Stop(hservo->htim, hservo->channel);
    }

    if (hservo->gpioPort != NULL) {
        HAL_GPIO_DeInit(hservo->gpioPort, hservo->gpioPin);
    }
}

SERVO_StatusTypeDef SERVO_PWM_SetPulseWidth(SERVO_Handle_t *hservo, uint16_t pulseWidthUs)
{
    if (pulseWidthUs > SERVO_PWM_STEPS_PER_PERIOD) {
        return SERVO_INVALID_PARAM;
    }

    TIM_PWM_SetDuty(hservo->htim, hservo->channel, pulseWidthUs);

    return SERVO_OK;
}
