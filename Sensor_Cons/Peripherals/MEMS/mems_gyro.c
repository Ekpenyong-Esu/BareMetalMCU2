/**
 ******************************************************************************
 * @file    mems_gyro.c
 * @brief   Gyroscope configuration and sample acquisition
 ******************************************************************************
 */

#include "mems_gyro.h"
#include "mems_io.h"
#include "mems_convert.h"
#include "mems_l3gd20.h"

#define MEMS_AXES_DATA_LENGTH 6U
#define MEMS_CONFIG_SETTLE_MS 10U

static const uint8_t s_odrBits[MEMS_GYRO_ODR_COUNT] = {
    [MEMS_GYRO_ODR_95Hz] = L3GD20_ODR_95Hz,
    [MEMS_GYRO_ODR_190Hz] = L3GD20_ODR_190Hz,
    [MEMS_GYRO_ODR_380Hz] = L3GD20_ODR_380Hz,
    [MEMS_GYRO_ODR_760Hz] = L3GD20_ODR_760Hz,
};

static const uint8_t s_bandwidthBits[MEMS_GYRO_BANDWIDTH_COUNT] = {
    [MEMS_GYRO_BANDWIDTH_1] = L3GD20_BANDWIDTH_1,
    [MEMS_GYRO_BANDWIDTH_2] = L3GD20_BANDWIDTH_2,
    [MEMS_GYRO_BANDWIDTH_3] = L3GD20_BANDWIDTH_3,
    [MEMS_GYRO_BANDWIDTH_4] = L3GD20_BANDWIDTH_4,
};

static const uint8_t s_fullScaleBits[MEMS_GYRO_FULLSCALE_COUNT] = {
    [MEMS_GYRO_FULLSCALE_250] = L3GD20_FULLSCALE_250,
    [MEMS_GYRO_FULLSCALE_500] = L3GD20_FULLSCALE_500,
    [MEMS_GYRO_FULLSCALE_2000] = L3GD20_FULLSCALE_2000,
};

static uint8_t MEMS_AxisMask(const MEMS_GyroConfigTypeDef *config) {
    uint8_t mask = 0U;

    if (config->XAxisEnable) {
        mask |= L3GD20_AXIS_X_ENABLE;
    }
    if (config->YAxisEnable) {
        mask |= L3GD20_AXIS_Y_ENABLE;
    }
    if (config->ZAxisEnable) {
        mask |= L3GD20_AXIS_Z_ENABLE;
    }

    return mask;
}

MEMS_StatusTypeDef MEMS_GyroApplyConfig(MEMS_HandleTypeDef *hmems,
                                        const MEMS_GyroConfigTypeDef *config) {
    MEMS_StatusTypeDef status = MEMS_OK;
    uint8_t ctrl_reg1 = 0;
    uint8_t ctrl_reg4 = 0;

    if (hmems == NULL || config == NULL) {
        return MEMS_INVALID_PARAM;
    }
    if (config->OutputDataRate >= MEMS_GYRO_ODR_COUNT ||
        config->Bandwidth >= MEMS_GYRO_BANDWIDTH_COUNT ||
        config->FullScale >= MEMS_GYRO_FULLSCALE_COUNT) {
        return MEMS_INVALID_PARAM;
    }

    ctrl_reg1 = (uint8_t)(s_odrBits[config->OutputDataRate] | s_bandwidthBits[config->Bandwidth] |
                          MEMS_AxisMask(config));
    if (!config->PowerDownMode) {
        ctrl_reg1 |= L3GD20_NORMAL_MODE;
    }

    ctrl_reg4 = s_fullScaleBits[config->FullScale];

    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG1_ADDR, ctrl_reg1);
    if (status != MEMS_OK) {
        return status;
    }

    status = MEMS_WriteRegister(hmems, L3GD20_CTRL_REG4_ADDR, ctrl_reg4);
    if (status != MEMS_OK) {
        return status;
    }

    hmems->GyroConfig = *config;

    HAL_Delay(MEMS_CONFIG_SETTLE_MS);

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_GyroConfig(MEMS_HandleTypeDef *hmems,
                                   const MEMS_GyroConfigTypeDef *config) {
    MEMS_StatusTypeDef status = MEMS_CheckReady(hmems);

    if (status != MEMS_OK) {
        return status;
    }

    return MEMS_GyroApplyConfig(hmems, config);
}

MEMS_StatusTypeDef MEMS_GyroReadRaw(MEMS_HandleTypeDef *hmems, MEMS_AxesRawTypeDef *axes) {
    MEMS_StatusTypeDef status = MEMS_CheckReady(hmems);
    uint8_t data[MEMS_AXES_DATA_LENGTH];

    if (status != MEMS_OK) {
        return status;
    }
    if (axes == NULL) {
        return MEMS_INVALID_PARAM;
    }

    status = MEMS_ReadRegisters(hmems, L3GD20_OUT_X_L_ADDR, data, MEMS_AXES_DATA_LENGTH);
    if (status != MEMS_OK) {
        return status;
    }

    axes->X = (int16_t)(((uint16_t)data[1] << 8) | data[0]);
    axes->Y = (int16_t)(((uint16_t)data[3] << 8) | data[2]);
    axes->Z = (int16_t)(((uint16_t)data[5] << 8) | data[4]);

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_GyroRead(MEMS_HandleTypeDef *hmems, MEMS_AxesTypeDef *axes) {
    MEMS_StatusTypeDef status = MEMS_OK;
    MEMS_AxesRawTypeDef raw_data;

    if (axes == NULL) {
        return MEMS_INVALID_PARAM;
    }

    status = MEMS_GyroReadRaw(hmems, &raw_data);
    if (status != MEMS_OK) {
        return status;
    }

    axes->X = MEMS_ConvertToDPS(raw_data.X, hmems->GyroConfig.FullScale);
    axes->Y = MEMS_ConvertToDPS(raw_data.Y, hmems->GyroConfig.FullScale);
    axes->Z = MEMS_ConvertToDPS(raw_data.Z, hmems->GyroConfig.FullScale);

    if (hmems->IsCalibrated) {
        axes->X -= hmems->CalibrationOffset.X;
        axes->Y -= hmems->CalibrationOffset.Y;
        axes->Z -= hmems->CalibrationOffset.Z;
    }

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_SetAxisEnable(MEMS_HandleTypeDef *hmems, uint8_t axis_mask) {
    MEMS_StatusTypeDef status = MEMS_CheckReady(hmems);

    if (status != MEMS_OK) {
        return status;
    }

    status = MEMS_UpdateRegister(hmems, L3GD20_CTRL_REG1_ADDR, L3GD20_AXES_ENABLE, axis_mask);
    if (status != MEMS_OK) {
        return status;
    }

    hmems->GyroConfig.XAxisEnable = (axis_mask & L3GD20_AXIS_X_ENABLE) != 0U;
    hmems->GyroConfig.YAxisEnable = (axis_mask & L3GD20_AXIS_Y_ENABLE) != 0U;
    hmems->GyroConfig.ZAxisEnable = (axis_mask & L3GD20_AXIS_Z_ENABLE) != 0U;

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_GetFullScale(MEMS_HandleTypeDef *hmems,
                                     MEMS_GyroFullScaleTypeDef *full_scale) {
    MEMS_StatusTypeDef status = MEMS_CheckReady(hmems);

    if (status != MEMS_OK) {
        return status;
    }
    if (full_scale == NULL) {
        return MEMS_INVALID_PARAM;
    }

    *full_scale = hmems->GyroConfig.FullScale;

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_SetPowerMode(MEMS_HandleTypeDef *hmems, bool power_down) {
    MEMS_StatusTypeDef status = MEMS_OK;

    if (hmems == NULL) {
        return MEMS_INVALID_PARAM;
    }

    status = MEMS_UpdateRegister(hmems, L3GD20_CTRL_REG1_ADDR, L3GD20_NORMAL_MODE,
                                 power_down ? 0U : L3GD20_NORMAL_MODE);
    if (status != MEMS_OK) {
        return status;
    }

    hmems->GyroConfig.PowerDownMode = power_down;

    return MEMS_OK;
}
