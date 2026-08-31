/**
 * @file    ili9341_io.c
 * @brief   ILI9341 SPI command and data transport implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "ili9341_io.h"

#include "spi.h"

/* Private constants ---------------------------------------------------------*/

#define ILI9341_MAX_READ_BYTES  4U

/* ST BSP uses 7 for this panel. */
#define ILI9341_SPI_CRC_POLY    7U

/* The on-board panel's slot on the shared bus. ST BSP expects 5.6-10 MHz;
   PCLK2/8 = 10.5 MHz is the closest the prescaler can get. */
static SPI_Device_t s_device;

SPI_StatusTypeDef ILI9341_IO_BusInit(void)
{
    SPI_ConfigTypeDef config = SPI_ConfigDefault();

    config.CRCPolynomial = ILI9341_SPI_CRC_POLY;

    return SPI_DeviceInit(&s_device, &config);
}

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

SPI_StatusTypeDef ili9341_WriteReg(uint8_t command)
{
    SPI_StatusTypeDef status = SPI_OK;

    ILI9341_SetCommandMode();
    ILI9341_Select();
    status = SPI_Transmit(&s_device, &command, 1U, SPI_TIMEOUT_LONG);
    ILI9341_Deselect();

    return status;
}

SPI_StatusTypeDef ili9341_WriteData(uint8_t data)
{
    SPI_StatusTypeDef status = SPI_OK;

    ILI9341_SetDataMode();
    ILI9341_Select();
    status = SPI_Transmit(&s_device, &data, 1U, SPI_TIMEOUT_LONG);
    ILI9341_Deselect();

    return status;
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

    if (SPI_Transmit(&s_device, &commandByte, 1U, SPI_TIMEOUT_LONG) != SPI_OK) {
        ILI9341_Deselect();
        return 0U;
    }

    ILI9341_SetDataMode();

    if (SPI_TransmitReceive(&s_device, transmitBuffer, responseBuffer, readSize,
                            SPI_TIMEOUT_LONG) != SPI_OK) {
        ILI9341_Deselect();
        return 0U;
    }

    ILI9341_Deselect();

    for (uint8_t index = 0U; index < readSize; index++) {
        value = (value << 8U) | responseBuffer[index];
    }

    return value;
}
