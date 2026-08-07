/**
 * @file ts_calibration.c
 * @brief Raw-to-display coordinate mapping and its calibration bounds
 */

#include "ts_calibration.h"

/* Measured raw bounds of the panel fitted to this board.
   Smaller raw X is the RIGHT edge and smaller raw Y is the BOTTOM edge, which is
   why the mapping below runs from the high display coordinate down to zero. */
#define TS_RAW_X_MIN    301     /*!< observed small raw X (right edge) */
#define TS_RAW_X_MAX    3796    /*!< observed large raw X (left edge) */
#define TS_RAW_Y_MIN    151     /*!< observed small raw Y (bottom edge) */
#define TS_RAW_Y_MAX    3605    /*!< observed large raw Y (top edge) */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Integer-safe linear mapping with input clamping, allows inverted output ranges
 * @param value Input value
 * @param inMin Input range start
 * @param inMax Input range end
 * @param outMin Output range start
 * @param outMax Output range end
 * @return int32_t Mapped value
 */
static int32_t TS_MapRange(int32_t value, int32_t inMin, int32_t inMax, int32_t outMin, int32_t outMax)
{
    if (inMax == inMin) {
        return outMin;
    }

    if (value < inMin) {
        value = inMin;
    } else if (value > inMax) {
        value = inMax;
    }

    int64_t scaled = (int64_t)(value - inMin) * (int64_t)(outMax - outMin);
    return (int32_t)((scaled / (int64_t)(inMax - inMin)) + outMin);
}

/**
 * @brief Clamp a mapped coordinate to the display
 * @param value Mapped coordinate
 * @param limit Display size along that axis
 * @return uint16_t Coordinate inside [0, limit - 1]
 */
static uint16_t TS_ClampToDisplay(int32_t value, int32_t limit)
{
    if (value < 0) {
        return 0;
    }
    if (value >= limit) {
        return (uint16_t)(limit - 1);
    }
    return (uint16_t)value;
}

/* Public functions ----------------------------------------------------------*/

void TS_GetDefaultCalibration(TS_CalibrationTypeDef *calibration)
{
    calibration->MinX = TS_RAW_X_MIN;
    calibration->MaxX = TS_RAW_X_MAX;
    calibration->MinY = TS_RAW_Y_MIN;
    calibration->MaxY = TS_RAW_Y_MAX;
    calibration->IsCalibrated = false;
}

TS_StatusTypeDef TS_SetCalibration(TS_HandleTypeDef *hts, TS_CalibrationTypeDef *calibration)
{
    if (hts == NULL || calibration == NULL) {
        return TS_INVALID_PARAM;
    }

    /* An empty span would make every touch collapse onto one display edge. */
    if (calibration->MinX >= calibration->MaxX || calibration->MinY >= calibration->MaxY) {
        return TS_INVALID_PARAM;
    }

    hts->Calibration = *calibration;
    hts->Calibration.IsCalibrated = true;

    return TS_OK;
}

TS_StatusTypeDef TS_GetCalibration(TS_HandleTypeDef *hts, TS_CalibrationTypeDef *calibration)
{
    if (hts == NULL || calibration == NULL) {
        return TS_INVALID_PARAM;
    }

    *calibration = hts->Calibration;
    return TS_OK;
}

TS_StatusTypeDef TS_MapToDisplay(const TS_HandleTypeDef *hts,
                                 uint16_t rawX, uint16_t rawY,
                                 uint16_t *dispX, uint16_t *dispY)
{
    if (hts == NULL || dispX == NULL || dispY == NULL) {
        return TS_INVALID_PARAM;
    }

    int32_t x = TS_MapRange((int32_t)rawX,
                            (int32_t)hts->Calibration.MinX, (int32_t)hts->Calibration.MaxX,
                            (int32_t)(TS_DISPLAY_WIDTH - 1), 0);

    int32_t y = TS_MapRange((int32_t)rawY,
                            (int32_t)hts->Calibration.MinY, (int32_t)hts->Calibration.MaxY,
                            (int32_t)(TS_DISPLAY_HEIGHT - 1), 0);

    *dispX = TS_ClampToDisplay(x, TS_DISPLAY_WIDTH);
    *dispY = TS_ClampToDisplay(y, TS_DISPLAY_HEIGHT);

    return TS_OK;
}
