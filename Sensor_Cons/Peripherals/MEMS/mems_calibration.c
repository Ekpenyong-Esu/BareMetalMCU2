/**
  ******************************************************************************
  * @file    mems_calibration.c
  * @brief   Gyroscope zero-rate offset calibration
  ******************************************************************************
  */

#include "mems_calibration.h"
#include "mems_gyro.h"
#include <string.h>

#define MEMS_CALIBRATION_SAMPLES_DEFAULT    100U
#define MEMS_CALIBRATION_SAMPLE_DELAY_MS    10U

MEMS_StatusTypeDef MEMS_CalibrateGyroscope(MEMS_HandleTypeDef *hmems, uint16_t samples)
{
    MEMS_StatusTypeDef status = MEMS_CheckReady(hmems);
    MEMS_AxesTypeDef sum = {0.0f, 0.0f, 0.0f};
    MEMS_AxesTypeDef reading;
    uint16_t valid_samples = 0U;

    if (status != MEMS_OK) {
        return status;
    }

    if (samples == 0U) {
        samples = MEMS_CALIBRATION_SAMPLES_DEFAULT;
    }

    /* Drop any previous calibration first: MEMS_GyroRead() subtracts the stored
       offset, so leaving it applied would measure only the residual bias, and an
       aborted run would keep reporting the old offset as valid. */
    hmems->IsCalibrated = false;
    memset(&hmems->CalibrationOffset, 0, sizeof(hmems->CalibrationOffset));

    for (uint16_t i = 0U; i < samples; i++) {
        if (MEMS_GyroRead(hmems, &reading) == MEMS_OK) {
            sum.X += reading.X;
            sum.Y += reading.Y;
            sum.Z += reading.Z;
            valid_samples++;
        }
        HAL_Delay(MEMS_CALIBRATION_SAMPLE_DELAY_MS);
    }

    if (valid_samples == 0U) {
        return MEMS_ERROR;
    }

    hmems->CalibrationOffset.X = sum.X / (float)valid_samples;
    hmems->CalibrationOffset.Y = sum.Y / (float)valid_samples;
    hmems->CalibrationOffset.Z = sum.Z / (float)valid_samples;
    hmems->IsCalibrated = true;

    return MEMS_OK;
}
