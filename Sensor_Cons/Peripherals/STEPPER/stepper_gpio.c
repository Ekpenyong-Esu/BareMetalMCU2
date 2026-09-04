/**
 ******************************************************************************
 * @file    stepper_gpio.c
 * @brief   Coil pin configuration and drive
 ******************************************************************************
 */

#include "stepper_gpio.h"
#include "gpio.h"
#include <stddef.h>

STEPPER_StatusTypeDef STEPPER_GPIO_Init(const STEPPER_Pins_t *pins) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (pins == NULL || pins->port1 == NULL || pins->port2 == NULL || pins->port3 == NULL ||
        pins->port4 == NULL) {
        return STEPPER_INVALID_PARAM;
    }

    const struct {
        GPIO_TypeDef *port;
        uint16_t pin;
    } coils[] = {
        {pins->port1, pins->pin1},
        {pins->port2, pins->pin2},
        {pins->port3, pins->pin3},
        {pins->port4, pins->pin4},
    };

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    for (size_t i = 0U; i < (sizeof(coils) / sizeof(coils[0])); i++) {
        GPIO_InitStruct.Pin = coils[i].pin;
        if (GPIO_Driver_Pin_Init(coils[i].port, &GPIO_InitStruct) != HAL_OK) {
            return STEPPER_ERROR;
        }
    }

    STEPPER_GPIO_ReleaseCoils(pins);

    return STEPPER_OK;
}

void STEPPER_GPIO_DeInit(const STEPPER_Pins_t *pins) {
    HAL_GPIO_DeInit(pins->port1, pins->pin1);
    HAL_GPIO_DeInit(pins->port2, pins->pin2);
    HAL_GPIO_DeInit(pins->port3, pins->pin3);
    HAL_GPIO_DeInit(pins->port4, pins->pin4);
}

void STEPPER_GPIO_ApplyPattern(const STEPPER_Pins_t *pins, const uint8_t *pattern) {
    HAL_GPIO_WritePin(pins->port1, pins->pin1, pattern[0] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port2, pins->pin2, pattern[1] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port3, pins->pin3, pattern[2] ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port4, pins->pin4, pattern[3] ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void STEPPER_GPIO_ReleaseCoils(const STEPPER_Pins_t *pins) {
    HAL_GPIO_WritePin(pins->port1, pins->pin1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port2, pins->pin2, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port3, pins->pin3, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(pins->port4, pins->pin4, GPIO_PIN_RESET);
}
