/**
  ******************************************************************************
  * @file    qspi_hw.c
  * @brief   Board wiring and SPI transport bring-up for the serial NOR flash
  ******************************************************************************
  */

#include "qspi_hw.h"
#include "gpio.h"

/** Chip-select settling loop; the flash needs a few ns of CS setup time. */
#define QSPI_CS_SETTLE_LOOPS            10

/* Single flash on SPI1, so a single transport handle. */
static SPI_HandleTypeDef s_hspi;

QSPI_StatusTypeDef QSPI_HW_InitGPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO driver enables the port clock */
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    GPIO_InitStruct.Pin = QSPI_SCK_PIN | QSPI_MISO_PIN | QSPI_MOSI_PIN;
    GPIO_Driver_Pin_Init(QSPI_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = QSPI_NCS_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Alternate = 0;
    GPIO_Driver_Pin_Init(QSPI_GPIO_PORT, &GPIO_InitStruct);

    QSPI_ChipSelect(false);

    return QSPI_OK;
}

QSPI_StatusTypeDef QSPI_HW_InitSPI(QSPI_HandleStructTypeDef *hqspi)
{
    if (hqspi == NULL) {
        return QSPI_INVALID_PARAM;
    }

    __HAL_RCC_SPI1_CLK_ENABLE();

    s_hspi.Instance = SPI1;
    s_hspi.Init.Mode = SPI_MODE_MASTER;
    s_hspi.Init.Direction = SPI_DIRECTION_2LINES;
    s_hspi.Init.DataSize = SPI_DATASIZE_8BIT;
    s_hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
    s_hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
    s_hspi.Init.NSS = SPI_NSS_SOFT;
    s_hspi.Init.BaudRatePrescaler = hqspi->Config.BaudRatePrescaler;
    s_hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
    s_hspi.Init.TIMode = SPI_TIMODE_DISABLE;
    s_hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    s_hspi.Init.CRCPolynomial = 10;

    if (HAL_SPI_Init(&s_hspi) != HAL_OK) {
        return QSPI_ERROR;
    }

    hqspi->hspi = &s_hspi;
    return QSPI_OK;
}

void QSPI_HW_DeInitSPI(QSPI_HandleStructTypeDef *hqspi)
{
    if (hqspi != NULL && hqspi->hspi != NULL) {
        HAL_SPI_DeInit(hqspi->hspi);
        hqspi->hspi = NULL;
    }
}

void QSPI_ChipSelect(bool select)
{
    HAL_GPIO_WritePin(QSPI_GPIO_PORT, QSPI_NCS_PIN, select ? GPIO_PIN_RESET : GPIO_PIN_SET);

    for (volatile int i = 0; i < QSPI_CS_SETTLE_LOOPS; i++) { }
}
