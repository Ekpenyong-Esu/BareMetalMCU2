/**
  ******************************************************************************
  * @file    stepper_gpio.c
  * @brief   Coil pin configuration and drive
  ******************************************************************************
  */

#include "stepper_gpio.h"
#include "gpio.h"

STEPPER_StatusTypeDef STEPPER_GPIO_Init(const STEPPER_Pins_t *pins)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (pins == NULL || pins->port1 == NULL || pins->port2 == NULL ||
        pins->port3 == NULL || pins->port4 == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    GPIO_InitStruct.Pin = pins->pin1;
    GPIO_Driver_Pin_Init(pins->port1, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = pins->pin2;
    GPIO_Driver_Pin_Init(pins->port2, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = pins->pin3;
    GPIO_Driver_Pin_Init(pins->port3, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = pins->pin4;
    GPIO_Driver_Pin_Init(pins->port4, &GPIO_InitStruct);

    STEPPER_GPIO_ReleaseCoils(pins);

    return STEPPER_OK;
}

void STEPPER_GPIO_DeInit(const STEPPER_Pins_t *pins)
{
    HAL_GPIO_DeInit(pins->port1, pins->pin1);
    HAL_GPIO_DeInit(pins->port2, pins->pin2);
    HAL_GPIO_DeInit(pins->port3, pins->pin3);
    HAL_GPIO_DeInit(pins->port4, pins->pin4);
}

void STEPPER_GPIO_ApplyPattern(const STEPPER_Pins_t *pins, const uint8_t *pattern)
{
    HAL_GPIO_WritePin(pins->port1, pins->pin1, pattern[0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port2, pins->pin2, pattern[1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port3, pins->pin3, pattern[2] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port4, pins->pin4, pattern[3] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void STEPPER_GPIO_ReleaseCoils(const STEPPER_Pins_t *pins)
{
    HAL_GPIO_WritePin(pins->port1, pins->pin1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port2, pins->pin2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port3, pins->pin3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port4, pins->pin4, GPIO_PIN_RESET);
}
