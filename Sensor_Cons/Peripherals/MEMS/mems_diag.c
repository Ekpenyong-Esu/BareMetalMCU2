/**
  ******************************************************************************
  * @file    mems_diag.c
  * @brief   Device identity, status, temperature and self-test
  ******************************************************************************
  */

#include "mems_diag.h"
#include "mems_io.h"
#include "mems_gyro.h"
#include "mems_l3gd20.h"

#define MEMS_DEVICE_VERSION             1U
#define MEMS_SELF_TEST_SETTLE_MS        100U

static bool MEMS_SelfTestShiftValid(int32_t diff)
{
    int32_t magnitude = (diff < 0) ? -diff : diff;

    return (magnitude > MEMS_SELF_TEST_DIFF_MIN) && (magnitude < MEMS_SELF_TEST_DIFF_MAX);
}

MEMS_StatusTypeDef MEMS_GetDeviceInfo(MEMS_HandleTypeDef *hmems, MEMS_DeviceInfoTypeDef *info)
{
    MEMS_StatusTypeDef status;

    if (hmems == NULL || info == NULL) {
        return MEMS_INVALID_PARAM;
    }

    status = MEMS_ReadRegister(hmems, L3GD20_WHO_AM_I_ADDR, &info->WhoAmI);
    if (status != MEMS_OK) {
        return status;
    }

    info->DeviceName = L3GD20_DEVICE_NAME;
    info->IsPresent = (info->WhoAmI == L3GD20_WHO_AM_I_VALUE);
    info->Version = MEMS_DEVICE_VERSION;

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_ReadStatus(MEMS_HandleTypeDef *hmems, uint8_t *status_reg)
{
    MEMS_StatusTypeDef status = MEMS_CheckReady(hmems);

    if (status != MEMS_OK) {
        return status;
    }
    if (status_reg == NULL) {
        return MEMS_INVALID_PARAM;
    }

    return MEMS_ReadRegister(hmems, L3GD20_STATUS_REG_ADDR, status_reg);
}

MEMS_StatusTypeDef MEMS_ReadTemperature(MEMS_HandleTypeDef *hmems, float *temperature)
{
    MEMS_StatusTypeDef status = MEMS_CheckReady(hmems);
    uint8_t temp_raw = 0U;

    if (status != MEMS_OK) {
        return status;
    }
    if (temperature == NULL) {
        return MEMS_INVALID_PARAM;
    }

    status = MEMS_ReadRegister(hmems, L3GD20_OUT_TEMP_ADDR, &temp_raw);
    if (status != MEMS_OK) {
        return status;
    }

    *temperature = MEMS_TEMPERATURE_OFFSET + (float)((int8_t)temp_raw);

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_SelfTest(MEMS_HandleTypeDef *hmems, bool *result)
{
    MEMS_StatusTypeDef status = MEMS_CheckReady(hmems);
    MEMS_StatusTypeDef disable_status;
    MEMS_AxesRawTypeDef data_normal;
    MEMS_AxesRawTypeDef data_test;

    if (status != MEMS_OK) {
        return status;
    }
    if (result == NULL) {
        return MEMS_INVALID_PARAM;
    }

    *result = false;

    status = MEMS_GyroReadRaw(hmems, &data_normal);
    if (status != MEMS_OK) {
        return status;
    }

    status = MEMS_UpdateRegister(hmems, L3GD20_CTRL_REG4_ADDR,
                                 L3GD20_CTRL_REG4_SELF_TEST, L3GD20_CTRL_REG4_SELF_TEST);
    if (status != MEMS_OK) {
        return status;
    }

    HAL_Delay(MEMS_SELF_TEST_SETTLE_MS);

    /* From here the device is in self-test mode, so every exit path must go
       through the disable below - leaving it on would corrupt all later reads. */
    status = MEMS_GyroReadRaw(hmems, &data_test);

    disable_status = MEMS_UpdateRegister(hmems, L3GD20_CTRL_REG4_ADDR,
                                         L3GD20_CTRL_REG4_SELF_TEST, 0U);

    if (status != MEMS_OK) {
        return status;
    }
    if (disable_status != MEMS_OK) {
        return disable_status;
    }

    *result = MEMS_SelfTestShiftValid((int32_t)data_test.X - (int32_t)data_normal.X) &&
              MEMS_SelfTestShiftValid((int32_t)data_test.Y - (int32_t)data_normal.Y) &&
              MEMS_SelfTestShiftValid((int32_t)data_test.Z - (int32_t)data_normal.Z);

    return MEMS_OK;
}
