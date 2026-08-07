/**
  ******************************************************************************
  * @file    ultrasonic_gpio.c
  * @brief   Trigger and echo pin handling (internal)
  ******************************************************************************
  */

#include "ultrasonic_gpio.h"
#include "gpio.h"
#include "log.h"

/**
 * @brief   Resolve the alternate function connecting a pin to a capture timer.
 * @retval  false when the timer has no alternate function mapping, which would
 *          otherwise leave the echo pin on AF0 and silently unconnected.
 */
static bool ULTRASONIC_GPIO_ResolveAlternate(const TIM_TypeDef *instance, uint8_t *alternate)
{
    if (instance == TIM1)       { *alternate = GPIO_AF1_TIM1;  }
    else if (instance == TIM2)  { *alternate = GPIO_AF1_TIM2;  }
    else if (instance == TIM3)  { *alternate = GPIO_AF2_TIM3;  }
    else if (instance == TIM4)  { *alternate = GPIO_AF2_TIM4;  }
    else if (instance == TIM5)  { *alternate = GPIO_AF2_TIM5;  }
    else if (instance == TIM8)  { *alternate = GPIO_AF3_TIM8;  }
    else if (instance == TIM9)  { *alternate = GPIO_AF3_TIM9;  }
    else if (instance == TIM10) { *alternate = GPIO_AF3_TIM10; }
    else if (instance == TIM11) { *alternate = GPIO_AF3_TIM11; }
    else if (instance == TIM12) { *alternate = GPIO_AF9_TIM12; }
    else if (instance == TIM13) { *alternate = GPIO_AF9_TIM13; }
    else if (instance == TIM14) { *alternate = GPIO_AF9_TIM14; }
    else { return false; }

    return true;
}

ULTRASONIC_StatusTypeDef ULTRASONIC_GPIO_Init(const ULTRASONIC_Pins_t *pins,
                                              const TIM_TypeDef *instance)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    uint8_t alternate;

    if (pins == NULL || pins->triggerPort == NULL || pins->echoPort == NULL ||
        instance == NULL) {
        return ULTRASONIC_INVALID_PARAM;
    }

    if (!ULTRASONIC_GPIO_ResolveAlternate(instance, &alternate)) {
        log_error("ULTRASONIC: timer has no input capture alternate function");
        return ULTRASONIC_INVALID_PARAM;
    }

    GPIO_InitStruct.Pin = pins->triggerPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Driver_Pin_Init(pins->triggerPort, &GPIO_InitStruct);

    HAL_GPIO_WritePin(pins->triggerPort, pins->triggerPin, GPIO_PIN_RESET);

    GPIO_InitStruct.Pin = pins->echoPin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = alternate;
    GPIO_Driver_Pin_Init(pins->echoPort, &GPIO_InitStruct);

    return ULTRASONIC_OK;
}

void ULTRASONIC_GPIO_DeInit(const ULTRASONIC_Pins_t *pins)
{
    if (pins == NULL) {
        return;
    }

    if (pins->triggerPort != NULL) {
        HAL_GPIO_WritePin(pins->triggerPort, pins->triggerPin, GPIO_PIN_RESET);
        HAL_GPIO_DeInit(pins->triggerPort, pins->triggerPin);
    }

    if (pins->echoPort != NULL) {
        HAL_GPIO_DeInit(pins->echoPort, pins->echoPin);
    }
}

void ULTRASONIC_GPIO_SetTrigger(const ULTRASONIC_Pins_t *pins, bool level)
{
    if (pins == NULL || pins->triggerPort == NULL) {
        return;
    }

    HAL_GPIO_WritePin(pins->triggerPort, pins->triggerPin,
                      level ? GPIO_PIN_SET : GPIO_PIN_RESET);
}
