/**
  ******************************************************************************
  * @file    accel_core.c
  * @brief   Lifecycle and identity for the MMA8452Q accelerometer
  ******************************************************************************
  */

#include "accel_core.h"
#include "accel_config.h"
#include "accel_io.h"
#include "accel_mma8452q.h"

static uint8_t s_cachedRange = ACCEL_RANGE_2G;

uint8_t ACCEL_GetCachedRange(void)
{
    return s_cachedRange;
}

void ACCEL_CacheRange(uint8_t range)
{
    s_cachedRange = range;
}

ACCEL_StatusTypeDef ACCEL_GetDeviceID(uint8_t *deviceId)
{
    if (deviceId == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    return ACCEL_ReadRegister(ACCEL_REG_WHO_AM_I, deviceId);
}

ACCEL_StatusTypeDef ACCEL_IsReady(void)
{
    uint8_t deviceId = 0;

    ACCEL_StatusTypeDef status = ACCEL_GetDeviceID(&deviceId);
    if (status != ACCEL_OK)
    {
        return status;
    }

    return (deviceId == ACCEL_DEVICE_ID) ? ACCEL_OK : ACCEL_NOT_READY;
}

ACCEL_StatusTypeDef ACCEL_Init_Custom(const ACCEL_ConfigTypeDef *config)
{
    if (config == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    ACCEL_StatusTypeDef status = ACCEL_IO_Init();
    if (status != ACCEL_OK)
    {
        return status;
    }

    status = ACCEL_IsReady();
    if (status != ACCEL_OK)
    {
        return status;
    }

    /* Range and data rate are only writable while the device is in standby. */
    status = ACCEL_WriteRegister(ACCEL_REG_CTRL_REG1, 0x00U);
    if (status != ACCEL_OK)
    {
        return status;
    }

    status = ACCEL_SetRange(config->Range);
    if (status != ACCEL_OK)
    {
        return status;
    }

    const uint8_t ctrlReg1 = (uint8_t)((config->DataRate << ACCEL_CTRL_REG1_ODR_SHIFT) |
                                       (config->LowNoise ? ACCEL_CTRL_REG1_LNOISE : 0x00U));
    status = ACCEL_WriteRegister(ACCEL_REG_CTRL_REG1, ctrlReg1);
    if (status != ACCEL_OK)
    {
        return status;
    }

    if (config->HighPassFilter)
    {
        status = ACCEL_EnableHighPassFilter(true);
        if (status != ACCEL_OK)
        {
            return status;
        }
    }

    return ACCEL_SetMode(config->Mode);
}

ACCEL_StatusTypeDef ACCEL_Init(void)
{
    const ACCEL_ConfigTypeDef defaultConfig = {
        .DataRate = ACCEL_ODR_100HZ,
        .Range = ACCEL_RANGE_2G,
        .Mode = ACCEL_MODE_ACTIVE,
        .HighPassFilter = false,
        .LowNoise = false
    };

    return ACCEL_Init_Custom(&defaultConfig);
}

ACCEL_StatusTypeDef ACCEL_DeInit(void)
{
    return ACCEL_SetMode(ACCEL_MODE_STANDBY);
}
