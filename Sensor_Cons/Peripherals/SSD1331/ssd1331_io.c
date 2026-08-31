/**
  ******************************************************************************
  * @file    ssd1331_io.c
  * @brief   SSD1331 SPI transport - internal to the driver
  ******************************************************************************
  */

#include "ssd1331_io.h"
#include "gpio.h"
#include "log.h"
#include "spi.h"
#include <string.h>

#define SSD1331_IO_CMD_MAX        32U
#define SSD1331_IO_RESET_MS       10U

/* The panel needs a 150 ns clock cycle at worst, so SPI5 has to run slower
   than the bus default: 84 MHz / 16 = 5.25 MHz. */
#define SSD1331_IO_PRESCALER      SPI_BAUDRATEPRESCALER_16
#define SSD1331_IO_CRC_POLYNOMIAL 10U

/* This panel's slot on the shared bus; it runs slower than the other devices. */
static SPI_Device_t s_device;

static SSD1331_StatusTypeDef SSD1331_IO_PinInit(GPIO_TypeDef *port, uint16_t pin)
{
    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.Pin = pin;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Pull = GPIO_NOPULL;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;

    return (GPIO_Driver_Pin_Init(port, &gpioInit) == HAL_OK) ? SSD1331_OK : SSD1331_ERROR;
}

static void SSD1331_IO_Select(const SSD1331_Config_t *config)
{
    GPIO_Driver_WritePin(config->csPort, config->csPin, GPIO_PIN_RESET);
}

static void SSD1331_IO_Deselect(const SSD1331_Config_t *config)
{
    GPIO_Driver_WritePin(config->csPort, config->csPin, GPIO_PIN_SET);
}

static void SSD1331_IO_SetCommandMode(const SSD1331_Config_t *config)
{
    GPIO_Driver_WritePin(config->dcPort, config->dcPin, GPIO_PIN_RESET);
}

static void SSD1331_IO_SetDataMode(const SSD1331_Config_t *config)
{
    GPIO_Driver_WritePin(config->dcPort, config->dcPin, GPIO_PIN_SET);
}

static void SSD1331_IO_Reset(const SSD1331_Config_t *config)
{
    if (config->rstPort == NULL) {
        return;
    }

    GPIO_Driver_WritePin(config->rstPort, config->rstPin, GPIO_PIN_SET);
    HAL_Delay(SSD1331_IO_RESET_MS);
    GPIO_Driver_WritePin(config->rstPort, config->rstPin, GPIO_PIN_RESET);
    HAL_Delay(SSD1331_IO_RESET_MS);
    GPIO_Driver_WritePin(config->rstPort, config->rstPin, GPIO_PIN_SET);
    HAL_Delay(SSD1331_IO_RESET_MS);
}

static SSD1331_StatusTypeDef SSD1331_IO_BusInit(void)
{
    SPI_ConfigTypeDef busConfig = SPI_ConfigDefault();

    busConfig.BaudRatePrescaler = SSD1331_IO_PRESCALER;
    busConfig.CRCPolynomial = SSD1331_IO_CRC_POLYNOMIAL;

    return (SPI_DeviceInit(&s_device, &busConfig) == SPI_OK) ? SSD1331_OK : SSD1331_ERROR;
}

SSD1331_StatusTypeDef SSD1331_IO_Init(const SSD1331_Config_t *config)
{
    SSD1331_StatusTypeDef status;

    if (config == NULL || config->csPort == NULL || config->dcPort == NULL) {
        return SSD1331_INVALID_PARAM;
    }

    status = SSD1331_IO_PinInit(config->csPort, config->csPin);
    if (status != SSD1331_OK) {
        log_error("SSD1331: chip-select pin init failed");
        return status;
    }

    status = SSD1331_IO_PinInit(config->dcPort, config->dcPin);
    if (status != SSD1331_OK) {
        log_error("SSD1331: data/command pin init failed");
        return status;
    }

    if (config->rstPort != NULL) {
        status = SSD1331_IO_PinInit(config->rstPort, config->rstPin);
        if (status != SSD1331_OK) {
            log_error("SSD1331: reset pin init failed");
            return status;
        }
    }

    /* Idle high before the bus comes up, so no stray edge is latched. */
    SSD1331_IO_Deselect(config);

    status = SSD1331_IO_BusInit();
    if (status != SSD1331_OK) {
        log_error("SSD1331: SPI bus init failed");
        return status;
    }

    SSD1331_IO_Reset(config);

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_IO_WriteCommand(const SSD1331_Config_t *config, uint8_t command)
{
    uint8_t frame = command;
    SPI_StatusTypeDef spiStatus;

    SSD1331_IO_SetCommandMode(config);
    SSD1331_IO_Select(config);
    spiStatus = SPI_Transmit(&s_device, &frame, 1U, SPI_TIMEOUT_DEFAULT);
    SSD1331_IO_Deselect(config);

    if (spiStatus != SPI_OK) {
        log_error("SSD1331: failed to write command 0x%02X", command);
        return SSD1331_ERROR;
    }

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_IO_WriteCommands(const SSD1331_Config_t *config,
                                               const uint8_t *commands, uint16_t count)
{
    uint8_t frame[SSD1331_IO_CMD_MAX];
    SPI_StatusTypeDef spiStatus;

    if (commands == NULL || count == 0U || count > SSD1331_IO_CMD_MAX) {
        return SSD1331_INVALID_PARAM;
    }

    /* Staged into a mutable copy: the SPI driver takes a non-const buffer. */
    memcpy(frame, commands, count);

    SSD1331_IO_SetCommandMode(config);
    SSD1331_IO_Select(config);
    spiStatus = SPI_Transmit(&s_device, frame, count, SPI_TIMEOUT_DEFAULT);
    SSD1331_IO_Deselect(config);

    if (spiStatus != SPI_OK) {
        log_error("SSD1331: failed to write %u command bytes", (unsigned)count);
        return SSD1331_ERROR;
    }

    return SSD1331_OK;
}

SSD1331_StatusTypeDef SSD1331_IO_WriteData(const SSD1331_Config_t *config,
                                           uint8_t *data, uint16_t size)
{
    SPI_StatusTypeDef spiStatus;

    if (data == NULL || size == 0U) {
        return SSD1331_INVALID_PARAM;
    }

    SSD1331_IO_SetDataMode(config);
    SSD1331_IO_Select(config);
    spiStatus = SPI_Transmit(&s_device, data, size, SPI_TIMEOUT_LONG);
    SSD1331_IO_Deselect(config);

    if (spiStatus != SPI_OK) {
        log_error("SSD1331: failed to write %u bytes of data", (unsigned)size);
        return SSD1331_ERROR;
    }

    return SSD1331_OK;
}
