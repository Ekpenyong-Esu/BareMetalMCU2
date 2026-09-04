/**
 ******************************************************************************
 * @file    mems_hw.c
 * @brief   Chip select and interrupt inputs of one L3GD20
 ******************************************************************************
 */

#include "mems_hw.h"
#include "gpio.h"

static MEMS_StatusTypeDef MEMS_InitCsPin(GPIO_TypeDef *port, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    if (GPIO_Driver_Pin_Init(port, &GPIO_InitStruct) != HAL_OK) {
        return MEMS_ERROR;
    }

    /* Idle high before the first transfer, or the part sees a partial frame. */
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);

    return MEMS_OK;
}

/* A NULL port means the application left that interrupt line unconnected. */
static MEMS_StatusTypeDef MEMS_InitIntPin(GPIO_TypeDef *port, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (port == NULL) {
        return MEMS_OK;
    }

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    if (GPIO_Driver_Pin_Init(port, &GPIO_InitStruct) != HAL_OK) {
        return MEMS_ERROR;
    }

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_HW_InitGPIO(MEMS_HandleTypeDef *hmems) {
    MEMS_StatusTypeDef status = MEMS_OK;

    if (hmems == NULL || hmems->CS_Port == NULL || hmems->CS_Pin == 0U) {
        return MEMS_INVALID_PARAM;
    }

    /* GPIO driver enables the port clock for each configured port */
    status = MEMS_InitCsPin(hmems->CS_Port, hmems->CS_Pin);
    if (status != MEMS_OK) {
        return status;
    }

    status = MEMS_InitIntPin(hmems->INT1_Port, hmems->INT1_Pin);
    if (status != MEMS_OK) {
        return status;
    }

    return MEMS_InitIntPin(hmems->INT2_Port, hmems->INT2_Pin);
}

void MEMS_CS_High(MEMS_HandleTypeDef *hmems) {
    HAL_GPIO_WritePin(hmems->CS_Port, hmems->CS_Pin, GPIO_PIN_SET);
}

void MEMS_CS_Low(MEMS_HandleTypeDef *hmems) {
    HAL_GPIO_WritePin(hmems->CS_Port, hmems->CS_Pin, GPIO_PIN_RESET);
}
