/**
  ******************************************************************************
  * @file    nokia5110_io.c
  * @brief   SPI and control-line transport for the Nokia 5110 LCD
  ******************************************************************************
  */

#include "nokia5110_io.h"
#include "spi.h"
#include "gpio.h"
#include "log.h"
#include "stm32f4xx_hal.h"

/* Control lines */
#define NOKIA5110_RST_PIN        GPIO_PIN_1    /* PB1 - Reset */
#define NOKIA5110_RST_PORT       GPIOB
#define NOKIA5110_CE_PIN         GPIO_PIN_0    /* PB0 - Chip Enable */
#define NOKIA5110_CE_PORT        GPIOB
#define NOKIA5110_DC_PIN         GPIO_PIN_2    /* PB2 - Data/Command */
#define NOKIA5110_DC_PORT        GPIOB

#define NOKIA5110_SPI_TIMEOUT    1000U         /* SPI timeout in ms */
#define NOKIA5110_RESET_LOW_MS   10U
#define NOKIA5110_RESET_HIGH_MS  10U           /* Settle before the first command */

/* This panel's slot on the shared bus. */
static SPI_Device_t s_device;

static NOKIA5110_StatusTypeDef NOKIA5110_IO_Transmit(const uint8_t *data, uint16_t size,
                                                     GPIO_PinState dcState)
{
    SPI_StatusTypeDef status = SPI_OK;

    HAL_GPIO_WritePin(NOKIA5110_DC_PORT, NOKIA5110_DC_PIN, dcState);
    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_RESET);

    status = SPI_Transmit(&s_device, (uint8_t *)(uintptr_t)data, size, NOKIA5110_SPI_TIMEOUT);

    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_SET);

    if (status == SPI_TIMEOUT) {
        return NOKIA5110_TIMEOUT;
    }

    return (status == SPI_OK) ? NOKIA5110_OK : NOKIA5110_ERROR;
}

NOKIA5110_StatusTypeDef NOKIA5110_IO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO driver enables the port clock */
    GPIO_InitStruct.Pin = NOKIA5110_RST_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_Driver_Pin_Init(NOKIA5110_RST_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = NOKIA5110_CE_PIN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Driver_Pin_Init(NOKIA5110_CE_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = NOKIA5110_DC_PIN;
    GPIO_Driver_Pin_Init(NOKIA5110_DC_PORT, &GPIO_InitStruct);

    HAL_GPIO_WritePin(NOKIA5110_RST_PORT, NOKIA5110_RST_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(NOKIA5110_DC_PORT, NOKIA5110_DC_PIN, GPIO_PIN_RESET);

    const SPI_ConfigTypeDef spiConfig = SPI_ConfigDefault();
    if (SPI_DeviceInit(&s_device, &spiConfig) != SPI_OK) {
        log_error("NOKIA5110: SPI device registration failed");
        return NOKIA5110_ERROR;
    }

    return NOKIA5110_OK;
}

void NOKIA5110_IO_DeInit(void)
{
    HAL_GPIO_DeInit(NOKIA5110_RST_PORT, NOKIA5110_RST_PIN);
    HAL_GPIO_DeInit(NOKIA5110_CE_PORT, NOKIA5110_CE_PIN);
    HAL_GPIO_DeInit(NOKIA5110_DC_PORT, NOKIA5110_DC_PIN);
}

void NOKIA5110_IO_Reset(void)
{
    HAL_GPIO_WritePin(NOKIA5110_RST_PORT, NOKIA5110_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(NOKIA5110_RESET_LOW_MS);
    HAL_GPIO_WritePin(NOKIA5110_RST_PORT, NOKIA5110_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(NOKIA5110_RESET_HIGH_MS);
}

NOKIA5110_StatusTypeDef NOKIA5110_IO_WriteCommand(uint8_t cmd)
{
    return NOKIA5110_IO_Transmit(&cmd, 1U, GPIO_PIN_RESET);
}

NOKIA5110_StatusTypeDef NOKIA5110_IO_WriteData(const uint8_t *data, uint16_t size)
{
    if (data == NULL || size == 0U) {
        return NOKIA5110_INVALID_PARAM;
    }

    return NOKIA5110_IO_Transmit(data, size, GPIO_PIN_SET);
}
