/**
  ******************************************************************************
  * @file    accel_calibration.c
  * @brief   Offset calibration for the MMA8452Q
  ******************************************************************************
  */

#include "accel_calibration.h"
#include "accel_data.h"
#include "accel_io.h"
#include "accel_mma8452q.h"
#include "spi.h"
#include <stddef.h>

#define ACCEL_CALIBRATION_SAMPLES   100
#define ACCEL_CALIBRATION_DELAY_MS  10U
#define ACCEL_GRAVITY_1G_4G_RANGE   8192   /**< 1 g expressed in +/-4g counts */
#define ACCEL_OFFSET_LSB_PER_COUNT  8      /**< OFF_* registers step 8 raw counts */

static const uint8_t s_offsetRegisters[] = {
    ACCEL_REG_OFF_X, ACCEL_REG_OFF_Y, ACCEL_REG_OFF_Z
};

ACCEL_StatusTypeDef ACCEL_Calibrate(void)
{
    int32_t sum[3] = {0, 0, 0};
    int16_t raw[3] = {0, 0, 0};

    for (uint16_t i = 0; i < ACCEL_CALIBRATION_SAMPLES; i++)
    {
        ACCEL_StatusTypeDef status = ACCEL_ReadRawData(&raw[0], &raw[1], &raw[2]);
        if (status != ACCEL_OK)
        {
            return status;
        }

        sum[0] += raw[0];
        sum[1] += raw[1];
        sum[2] += raw[2];

        HAL_Delay(ACCEL_CALIBRATION_DELAY_MS);
    }

    const int32_t meanX = sum[0] / ACCEL_CALIBRATION_SAMPLES;
    const int32_t meanY = sum[1] / ACCEL_CALIBRATION_SAMPLES;
    const int32_t meanZ = sum[2] / ACCEL_CALIBRATION_SAMPLES;

    return ACCEL_SetOffset((int8_t)(-meanX / ACCEL_OFFSET_LSB_PER_COUNT),
                           (int8_t)(-meanY / ACCEL_OFFSET_LSB_PER_COUNT),
                           (int8_t)((ACCEL_GRAVITY_1G_4G_RANGE - meanZ) / ACCEL_OFFSET_LSB_PER_COUNT));
}

ACCEL_StatusTypeDef ACCEL_SetOffset(int8_t xOffset, int8_t yOffset, int8_t zOffset)
{
    const int8_t offsets[] = { xOffset, yOffset, zOffset };

    for (size_t i = 0; i < (sizeof(offsets) / sizeof(offsets[0])); i++)
    {
        ACCEL_StatusTypeDef status = ACCEL_WriteRegister(s_offsetRegisters[i], (uint8_t)offsets[i]);
        if (status != ACCEL_OK)
        {
            return status;
        }
    }

    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_GetOffset(int8_t *xOffset, int8_t *yOffset, int8_t *zOffset)
{
    if (xOffset == NULL || yOffset == NULL || zOffset == NULL)
    {
        return ACCEL_INVALID_PARAM;
    }

    int8_t *const offsets[] = { xOffset, yOffset, zOffset };

    for (size_t i = 0; i < (sizeof(offsets) / sizeof(offsets[0])); i++)
    {
        uint8_t value = 0;

        ACCEL_StatusTypeDef status = ACCEL_ReadRegister(s_offsetRegisters[i], &value);
        if (status != ACCEL_OK)
        {
            return status;
        }

        *offsets[i] = (int8_t)value;
    }

    return ACCEL_OK;
}
