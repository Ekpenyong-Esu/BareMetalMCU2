/**
 * @file    ili9341_io.c
 * @brief   ILI9341 SPI command and data transport implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "ili9341_io.h"

#include "spi.h"
#include "gpio.h"

/* Private constants ---------------------------------------------------------*/

#define ILI9341_MAX_READ_BYTES 4U

/* ST BSP uses 7 for this panel. */
#define ILI9341_SPI_CRC_POLY 7U

/* Private functions ---------------------------------------------------------*/

static void ILI9341_Select(const ILI9341_Config_t *config) {
    HAL_GPIO_WritePin(config->csPort, config->csPin, GPIO_PIN_RESET);
}

static void ILI9341_Deselect(const ILI9341_Config_t *config) {
    HAL_GPIO_WritePin(config->csPort, config->csPin, GPIO_PIN_SET);
}

static void ILI9341_SetCommandMode(const ILI9341_Config_t *config) {
    HAL_GPIO_WritePin(config->dcPort, config->dcPin, GPIO_PIN_RESET);
}

static void ILI9341_SetDataMode(const ILI9341_Config_t *config) {
    HAL_GPIO_WritePin(config->dcPort, config->dcPin, GPIO_PIN_SET);
}

static void ILI9341_ConfigureOutput(GPIO_TypeDef *port, uint16_t pin) {
    GPIO_InitTypeDef init = {0};

    init.Pin = pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Driver_Pin_Init(port, &init);
}

/** Hardware reset when the line is wired; otherwise the soft reset in the
    init sequence has to do. */
static void ILI9341_PulseReset(const ILI9341_Config_t *config) {
    if (config->rstPort == NULL) {
        return;
    }

    HAL_GPIO_WritePin(config->rstPort, config->rstPin, GPIO_PIN_RESET);
    HAL_Delay(ILI9341_RESET_LOW_MS);
    HAL_GPIO_WritePin(config->rstPort, config->rstPin, GPIO_PIN_SET);
    HAL_Delay(ILI9341_RESET_HIGH_MS);
}

/* Exported functions --------------------------------------------------------*/

SPI_StatusTypeDef ILI9341_IO_Init(ILI9341_Handle_t *hili) {
    const ILI9341_Config_t *config = NULL;
    SPI_ConfigTypeDef spiConfig;
    SPI_StatusTypeDef status = SPI_OK;

    if (hili == NULL) {
        return SPI_INVALID_PARAM;
    }

    config = &hili->config;
    if (config->bus == NULL || config->csPort == NULL || config->dcPort == NULL) {
        return SPI_INVALID_PARAM;
    }

    ILI9341_ConfigureOutput(config->dcPort, config->dcPin);
    ILI9341_ConfigureOutput(config->csPort, config->csPin);

    /* ST BSP toggles the select line once before the first transfer. */
    ILI9341_Select(config);
    ILI9341_Deselect(config);

    if (config->rstPort != NULL) {
        ILI9341_ConfigureOutput(config->rstPort, config->rstPin);
        HAL_GPIO_WritePin(config->rstPort, config->rstPin, GPIO_PIN_SET);
    }

    /* ST BSP expects 5.6-10 MHz; the default PCLK/8 is the closest the
       prescaler can get. */
    spiConfig = SPI_ConfigDefault();
    spiConfig.CRCPolynomial = ILI9341_SPI_CRC_POLY;

    status = SPI_DeviceInit(&hili->device, config->bus, &spiConfig);
    if (status != SPI_OK) {
        return status;
    }

    ILI9341_PulseReset(config);

    return SPI_OK;
}

void ILI9341_IO_DeInit(ILI9341_Handle_t *hili) {
    if (hili == NULL) {
        return;
    }

    HAL_GPIO_WritePin(hili->config.csPort, hili->config.csPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hili->config.dcPort, hili->config.dcPin, GPIO_PIN_RESET);
    /* The bus is the application's; leave it open for other devices. */
}

SPI_StatusTypeDef ILI9341_WriteReg(ILI9341_Handle_t *hili, uint8_t command) {
    SPI_StatusTypeDef status = SPI_OK;

    if (hili == NULL) {
        return SPI_INVALID_PARAM;
    }

    ILI9341_SetCommandMode(&hili->config);
    ILI9341_Select(&hili->config);
    status = SPI_Transmit(&hili->device, &command, 1U, SPI_TIMEOUT_LONG);
    ILI9341_Deselect(&hili->config);

    return status;
}

SPI_StatusTypeDef ILI9341_WriteData(ILI9341_Handle_t *hili, uint8_t data) {
    SPI_StatusTypeDef status = SPI_OK;

    if (hili == NULL) {
        return SPI_INVALID_PARAM;
    }

    ILI9341_SetDataMode(&hili->config);
    ILI9341_Select(&hili->config);
    status = SPI_Transmit(&hili->device, &data, 1U, SPI_TIMEOUT_LONG);
    ILI9341_Deselect(&hili->config);

    return status;
}

uint32_t ILI9341_ReadData(ILI9341_Handle_t *hili, uint16_t command, uint8_t readSize) {
    uint8_t responseBuffer[ILI9341_MAX_READ_BYTES] = {0};
    uint8_t transmitBuffer[ILI9341_MAX_READ_BYTES] = {0};
    uint8_t commandByte = (uint8_t)command;
    uint32_t value = 0U;

    if (hili == NULL) {
        return 0U;
    }

    if (readSize > ILI9341_MAX_READ_BYTES) {
        readSize = ILI9341_MAX_READ_BYTES;
    }

    /* Command and read phases share one chip-select assertion. */
    ILI9341_SetCommandMode(&hili->config);
    ILI9341_Select(&hili->config);

    if (SPI_Transmit(&hili->device, &commandByte, 1U, SPI_TIMEOUT_LONG) != SPI_OK) {
        ILI9341_Deselect(&hili->config);
        return 0U;
    }

    ILI9341_SetDataMode(&hili->config);

    if (SPI_TransmitReceive(&hili->device, transmitBuffer, responseBuffer, readSize,
                            SPI_TIMEOUT_LONG) != SPI_OK) {
        ILI9341_Deselect(&hili->config);
        return 0U;
    }

    ILI9341_Deselect(&hili->config);

    for (uint8_t index = 0U; index < readSize; index++) {
        value = (value << 8U) | responseBuffer[index];
    }

    return value;
}
