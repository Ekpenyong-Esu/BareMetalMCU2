/**
 * @file    ili9341_io.c
 * @brief   ILI9341 SPI command and data transport implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "ili9341_io.h"

#include "spi.h"

/* Private constants ---------------------------------------------------------*/

#define ILI9341_MAX_READ_BYTES  4U

/* Private functions ---------------------------------------------------------*/

static void ILI9341_Select(void)
{
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_RESET);
}

static void ILI9341_Deselect(void)
{
    HAL_GPIO_WritePin(ILI9341_CS_PORT, ILI9341_CS_PIN, GPIO_PIN_SET);
}

static void ILI9341_SetCommandMode(void)
{
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_RESET);
}

static void ILI9341_SetDataMode(void)
{
    HAL_GPIO_WritePin(ILI9341_WRX_PORT, ILI9341_WRX_PIN, GPIO_PIN_SET);
}

/* Exported functions --------------------------------------------------------*/

void ili9341_WriteReg(uint8_t command)
{
    ILI9341_SetCommandMode();
    ILI9341_Select();
    SPI_Transmit(&command, 1U, SPI_TIMEOUT_LONG);
    ILI9341_Deselect();
}

void ili9341_WriteData(uint8_t data)
{
    ILI9341_SetDataMode();
    ILI9341_Select();
    SPI_Transmit(&data, 1U, SPI_TIMEOUT_LONG);
    ILI9341_Deselect();
}

uint32_t ili9341_ReadData(uint16_t command, uint8_t readSize)
{
    uint8_t responseBuffer[ILI9341_MAX_READ_BYTES] = {0};
    uint8_t transmitBuffer[ILI9341_MAX_READ_BYTES] = {0};
    uint8_t commandByte = (uint8_t)command;
    uint32_t value = 0U;

    if (readSize > ILI9341_MAX_READ_BYTES) {
        readSize = ILI9341_MAX_READ_BYTES;
    }

    /* Command and read phases share one chip-select assertion. */
    ILI9341_SetCommandMode();
    ILI9341_Select();
    SPI_Transmit(&commandByte, 1U, SPI_TIMEOUT_LONG);

    ILI9341_SetDataMode();
    SPI_TransmitReceive(transmitBuffer, responseBuffer, readSize, SPI_TIMEOUT_LONG);
    ILI9341_Deselect();

    for (uint8_t index = 0U; index < readSize; index++) {
        value = (value << 8U) | responseBuffer[index];
    }

    return value;
}
