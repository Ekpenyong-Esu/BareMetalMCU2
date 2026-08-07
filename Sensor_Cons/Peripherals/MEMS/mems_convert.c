/**
  ******************************************************************************
  * @file    mems_convert.c
  * @brief   Raw-to-engineering-unit conversion for the L3GD20
  ******************************************************************************
  */

#include "mems_convert.h"
#include "mems_l3gd20.h"

#define MEMS_MDPS_PER_DPS               1000.0f

static const float s_sensitivity[MEMS_GYRO_FULLSCALE_COUNT] = {
    [MEMS_GYRO_FULLSCALE_250]  = L3GD20_SENSITIVITY_250DPS,
    [MEMS_GYRO_FULLSCALE_500]  = L3GD20_SENSITIVITY_500DPS,
    [MEMS_GYRO_FULLSCALE_2000] = L3GD20_SENSITIVITY_2000DPS,
};

float MEMS_ConvertToDPS(int16_t raw_data, MEMS_GyroFullScaleTypeDef full_scale)
{
    if (full_scale >= MEMS_GYRO_FULLSCALE_COUNT) {
        full_scale = MEMS_GYRO_FULLSCALE_250;
    }

    return ((float)raw_data * s_sensitivity[full_scale]) / MEMS_MDPS_PER_DPS;
}
