/**
  ******************************************************************************
  * @file    spi_core.c
  * @brief   Device registration and bus ownership for the SPI bus driver
  ******************************************************************************
  */

#include "spi_core.h"
#include "log.h"

#define SPI_CRC_POLYNOMIAL_DEFAULT    10U

/* The board exposes one usable SPI bus, so the peripheral handle is shared;
   what varies per device is the configuration loaded into it. */
static SPI_HandleTypeDef s_bus;
static SPI_Device_t *s_owner;

SPI_HandleTypeDef *SPI_GetHandle(void) { return &s_bus; }

SPI_StatusTypeDef SPI_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus) {
        case HAL_OK:      return SPI_OK;
        case HAL_ERROR:   return SPI_ERROR;
        case HAL_BUSY:    return SPI_BUSY;
        case HAL_TIMEOUT: return SPI_TIMEOUT;
        default:          return SPI_ERROR;
    }
}

SPI_ConfigTypeDef SPI_ConfigDefault(void)
{
    /* ST BSP settings for the on-board SPI5 devices: mode 0, MSB first, and
       PCLK2/8 = 10.5 MHz, which both the display and the gyro accept. */
    const SPI_ConfigTypeDef config = {
        .Mode              = SPI_MODE_MASTER,
        .Direction         = SPI_DIRECTION_2LINES,
        .DataSize          = SPI_DATASIZE_8BIT,
        .CLKPolarity       = SPI_POLARITY_LOW,
        .CLKPhase          = SPI_PHASE_1EDGE,
        .NSS               = SPI_NSS_SOFT,
        .BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8,
        .FirstBit          = SPI_FIRSTBIT_MSB,
        .TIMode            = SPI_TIMODE_DISABLE,
        .CRCCalculation    = SPI_CRCCALCULATION_DISABLE,
        .CRCPolynomial     = SPI_CRC_POLYNOMIAL_DEFAULT,
    };

    return config;
}

bool SPI_DeviceIsReady(const SPI_Device_t *device)
{
    return (device != NULL) && device->ready;
}

SPI_StatusTypeDef SPI_DeviceInit(SPI_Device_t *device, const SPI_ConfigTypeDef *config)
{
    if (device == NULL || config == NULL) {
        return SPI_INVALID_PARAM;
    }

    device->config = *config;
    device->ready = true;

    return SPI_OK;
}

SPI_StatusTypeDef SPI_Select(SPI_Device_t *device)
{
    if (!SPI_DeviceIsReady(device)) {
        return SPI_INVALID_PARAM;
    }

    if (s_owner == device) {
        return SPI_OK;
    }

    s_bus.Instance = SPI5;
    s_bus.Init.Mode = device->config.Mode;
    s_bus.Init.Direction = device->config.Direction;
    s_bus.Init.DataSize = device->config.DataSize;
    s_bus.Init.CLKPolarity = device->config.CLKPolarity;
    s_bus.Init.CLKPhase = device->config.CLKPhase;
    s_bus.Init.NSS = device->config.NSS;
    s_bus.Init.BaudRatePrescaler = device->config.BaudRatePrescaler;
    s_bus.Init.FirstBit = device->config.FirstBit;
    s_bus.Init.TIMode = device->config.TIMode;
    s_bus.Init.CRCCalculation = device->config.CRCCalculation;
    s_bus.Init.CRCPolynomial = device->config.CRCPolynomial;

    if (HAL_SPI_Init(&s_bus) != HAL_OK) {
        log_error("SPI: failed to program the bus for the selected device");
        s_owner = NULL;
        return SPI_ERROR;
    }

    s_owner = device;

    return SPI_OK;
}

SPI_StatusTypeDef SPI_BusDeInit(void)
{
    HAL_StatusTypeDef halStatus = HAL_SPI_DeInit(&s_bus);

    s_owner = NULL;

    return SPI_ConvertHALStatus(halStatus);
}
