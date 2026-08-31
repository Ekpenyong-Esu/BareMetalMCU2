/**
  ******************************************************************************
  * @file    i2c_core.c
  * @brief   Bus lifecycle and per-device registration for the I2C driver
  ******************************************************************************
  */

#include "i2c_core.h"
#include "log.h"

#define I2C_CLOCK_SPEED_STANDARD     100000U

/* The board exposes one usable I2C bus, so the peripheral handle is shared;
   what varies per device is the configuration loaded into it. */
static I2C_HandleTypeDef s_bus;
static I2C_Device_t *s_owner;

I2C_HandleTypeDef *I2C_GetHandle(void) { return &s_bus; }

I2C_StatusTypeDef I2C_ConvertHALStatus(HAL_StatusTypeDef halStatus)
{
    switch (halStatus) {
        case HAL_OK:      return I2C_OK;
        case HAL_ERROR:   return I2C_ERROR;
        case HAL_BUSY:    return I2C_BUSY;
        case HAL_TIMEOUT: return I2C_TIMEOUT;
        default:          return I2C_ERROR;
    }
}

I2C_ConfigTypeDef I2C_ConfigDefault(void)
{
    I2C_ConfigTypeDef config;

    config.ClockSpeed = I2C_CLOCK_SPEED_STANDARD;
    config.DutyCycle = I2C_DUTYCYCLE_2;
    config.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    config.OwnAddress1 = 0;
    config.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    config.OwnAddress2 = 0;
    config.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    config.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    return config;
}

bool I2C_DeviceIsReady(const I2C_Device_t *device)
{
    return (device != NULL) && device->ready;
}

I2C_StatusTypeDef I2C_DeviceInit(I2C_Device_t *device, uint16_t address,
                                 const I2C_ConfigTypeDef *config)
{
    if ((device == NULL) || (config == NULL)) {
        return I2C_INVALID_PARAM;
    }

    device->address = address;
    device->config = *config;
    device->ready = true;

    /* A registration replaces whatever the bus was told last time. */
    if (s_owner == device) {
        s_owner = NULL;
    }

    return I2C_OK;
}

I2C_StatusTypeDef I2C_Select(I2C_Device_t *device)
{
    if (!I2C_DeviceIsReady(device)) {
        return I2C_INVALID_PARAM;
    }

    if (s_owner == device) {
        return I2C_OK;
    }

    s_bus.Instance = I2C3;
    s_bus.Init.ClockSpeed = device->config.ClockSpeed;
    s_bus.Init.DutyCycle = device->config.DutyCycle;
    s_bus.Init.AddressingMode = device->config.AddressingMode;
    s_bus.Init.OwnAddress1 = device->config.OwnAddress1;
    s_bus.Init.DualAddressMode = device->config.DualAddressMode;
    s_bus.Init.OwnAddress2 = device->config.OwnAddress2;
    s_bus.Init.GeneralCallMode = device->config.GeneralCallMode;
    s_bus.Init.NoStretchMode = device->config.NoStretchMode;

    if (HAL_I2C_Init(&s_bus) != HAL_OK) {
        log_error("I2C: failed to configure the bus for a device");
        s_owner = NULL;
        return I2C_ERROR;
    }

    s_owner = device;

    return I2C_OK;
}

I2C_StatusTypeDef I2C_BusDeInit(void)
{
    HAL_StatusTypeDef halStatus = HAL_I2C_DeInit(&s_bus);

    /* Devices stay registered; the next transfer reprograms the bus. */
    s_owner = NULL;

    return I2C_ConvertHALStatus(halStatus);
}
