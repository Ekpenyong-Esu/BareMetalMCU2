/**
 ******************************************************************************
 * @file    accel_data.c
 * @brief   Acceleration sample acquisition
 ******************************************************************************
 */

#include "accel_data.h"
#include "accel_convert.h"
#include "accel_io.h"
#include "accel_mma8452q.h"

#define ACCEL_SAMPLE_BYTES 6U

ACCEL_StatusTypeDef ACCEL_ReadRawData(ACCEL_Handle_t *haccel, int16_t *xAxis, int16_t *yAxis,
                                      int16_t *zAxis) {
    if (xAxis == NULL || yAxis == NULL || zAxis == NULL) {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t buffer[ACCEL_SAMPLE_BYTES];

    ACCEL_StatusTypeDef status =
        ACCEL_ReadRegisters(haccel, ACCEL_REG_OUT_X_MSB, buffer, ACCEL_SAMPLE_BYTES);
    if (status != ACCEL_OK) {
        return status;
    }

    *xAxis = ACCEL_ConvertRawToSigned(buffer[0], buffer[1]);
    *yAxis = ACCEL_ConvertRawToSigned(buffer[2], buffer[3]);
    *zAxis = ACCEL_ConvertRawToSigned(buffer[4], buffer[5]);

    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_ReadData(ACCEL_Handle_t *haccel, ACCEL_DataTypeDef *data) {
    if (data == NULL) {
        return ACCEL_INVALID_PARAM;
    }

    ACCEL_StatusTypeDef status = ACCEL_ReadRawData(haccel, &data->X, &data->Y, &data->Z);
    if (status != ACCEL_OK) {
        return status;
    }

    const uint8_t range = haccel->cachedRange;

    data->X_g = ACCEL_ConvertToG(data->X, range);
    data->Y_g = ACCEL_ConvertToG(data->Y, range);
    data->Z_g = ACCEL_ConvertToG(data->Z, range);

    return ACCEL_OK;
}
