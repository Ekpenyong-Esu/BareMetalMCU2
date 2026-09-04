/**
 ******************************************************************************
 * @file    qspi_hw.c
 * @brief   Chip select and bus registration for the serial NOR flash
 ******************************************************************************
 */

#include "qspi_hw.h"
#include "gpio.h"

/** Chip-select settling loop; the flash needs a few ns of CS setup time. */
#define QSPI_CS_SETTLE_LOOPS 10

QSPI_StatusTypeDef QSPI_HW_InitCS(QSPI_HandleStructTypeDef *hqspi) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hqspi == NULL || hqspi->Config.csPort == NULL || hqspi->Config.csPin == 0U) {
        return QSPI_INVALID_PARAM;
    }

    /* GPIO driver enables the port clock */
    GPIO_InitStruct.Pin = hqspi->Config.csPin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    if (GPIO_Driver_Pin_Init(hqspi->Config.csPort, &GPIO_InitStruct) != HAL_OK) {
        return QSPI_ERROR;
    }

    QSPI_ChipSelect(hqspi, false);

    return QSPI_OK;
}

void QSPI_HW_DeInitCS(QSPI_HandleStructTypeDef *hqspi) {
    if (hqspi != NULL && hqspi->Config.csPort != NULL) {
        GPIO_Driver_Pin_DeInit(hqspi->Config.csPort, hqspi->Config.csPin);
    }
}

QSPI_StatusTypeDef QSPI_HW_RegisterDevice(QSPI_HandleStructTypeDef *hqspi) {
    if (hqspi == NULL) {
        return QSPI_INVALID_PARAM;
    }

    /* Mode 0, 8-bit, MSB first is what every JEDEC serial flash speaks;
       only the clock divider is the caller's choice. */
    SPI_ConfigTypeDef config = SPI_ConfigDefault();
    config.BaudRatePrescaler = hqspi->Config.BaudRatePrescaler;

    if (SPI_DeviceInit(&hqspi->device, hqspi->Config.bus, &config) != SPI_OK) {
        return QSPI_ERROR;
    }

    return QSPI_OK;
}

void QSPI_ChipSelect(const QSPI_HandleStructTypeDef *hqspi, bool select) {
    HAL_GPIO_WritePin(hqspi->Config.csPort, hqspi->Config.csPin,
                      select ? GPIO_PIN_RESET : GPIO_PIN_SET);

    for (volatile int i = 0; i < QSPI_CS_SETTLE_LOOPS; i++) {
    }
}
