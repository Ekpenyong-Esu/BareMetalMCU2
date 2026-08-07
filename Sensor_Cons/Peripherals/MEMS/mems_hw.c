/**
  ******************************************************************************
  * @file    mems_hw.c
  * @brief   Board wiring for the on-board L3GD20
  ******************************************************************************
  */

#include "mems_hw.h"
#include "gpio.h"

static GPIO_TypeDef *MEMS_CsPort(const MEMS_HandleTypeDef *hmems)
{
    if (hmems != NULL && hmems->CS_Port != NULL && hmems->CS_Pin != 0U) {
        return hmems->CS_Port;
    }
    return MEMS_CS_GPIO_PORT;
}

static uint16_t MEMS_CsPin(const MEMS_HandleTypeDef *hmems)
{
    if (hmems != NULL && hmems->CS_Port != NULL && hmems->CS_Pin != 0U) {
        return hmems->CS_Pin;
    }
    return MEMS_CS_PIN;
}

static void MEMS_InitCsPin(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Driver_Pin_Init(port, &GPIO_InitStruct);

    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
}

MEMS_StatusTypeDef MEMS_HW_InitGPIO(MEMS_HandleTypeDef *hmems)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO driver enables the port clock for each configured port */
    MEMS_InitCsPin(MEMS_CsPort(hmems), MEMS_CsPin(hmems));

    GPIO_InitStruct.Pin = MEMS_SPI_SCK_PIN | MEMS_SPI_MISO_PIN | MEMS_SPI_MOSI_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI5;
    GPIO_Driver_Pin_Init(MEMS_SPI_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MEMS_INT1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Alternate = 0;
    GPIO_Driver_Pin_Init(MEMS_INT1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = MEMS_INT2_PIN;
    GPIO_Driver_Pin_Init(MEMS_INT2_GPIO_PORT, &GPIO_InitStruct);

    return MEMS_OK;
}

void MEMS_SetCS(MEMS_HandleTypeDef *hmems, GPIO_TypeDef *csPort, uint16_t csPin)
{
    if (hmems == NULL) {
        return;
    }

    hmems->CS_Port = csPort;
    hmems->CS_Pin = csPin;

    if (csPort != NULL && csPin != 0U) {
        MEMS_InitCsPin(csPort, csPin);
    }
}

void MEMS_CS_High(MEMS_HandleTypeDef *hmems)
{
    HAL_GPIO_WritePin(MEMS_CsPort(hmems), MEMS_CsPin(hmems), GPIO_PIN_SET);
}

void MEMS_CS_Low(MEMS_HandleTypeDef *hmems)
{
    HAL_GPIO_WritePin(MEMS_CsPort(hmems), MEMS_CsPin(hmems), GPIO_PIN_RESET);
}
