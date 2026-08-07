/**
 * @file ts_calibration.h
 * @brief Raw-to-display coordinate mapping and its calibration bounds
 */

#ifndef TS_CALIBRATION_H
#define TS_CALIBRATION_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_types.h"

/**
 * @brief Fill in the raw bounds measured on this panel
 * @param calibration Destination
 */
void TS_GetDefaultCalibration(TS_CalibrationTypeDef *calibration);

/**
 * @brief Replace the calibration bounds
 * @param hts Touchscreen handle
 * @param calibration Raw ADC bounds measured by the application
 * @return TS_StatusTypeDef TS_INVALID_PARAM when a span is empty
 */
TS_StatusTypeDef TS_SetCalibration(TS_HandleTypeDef *hts, TS_CalibrationTypeDef *calibration);

/**
 * @brief Read back the calibration bounds in use
 * @param hts Touchscreen handle
 * @param calibration Destination
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_GetCalibration(TS_HandleTypeDef *hts, TS_CalibrationTypeDef *calibration);

/**
 * @brief Map a raw ADC sample onto display pixel coordinates
 * @param hts Touchscreen handle supplying the calibration bounds
 * @param rawX Raw X sample
 * @param rawY Raw Y sample
 * @param dispX Destination display X
 * @param dispY Destination display Y
 * @return TS_StatusTypeDef Status of the operation
 */
TS_StatusTypeDef TS_MapToDisplay(const TS_HandleTypeDef *hts,
                                 uint16_t rawX, uint16_t rawY,
                                 uint16_t *dispX, uint16_t *dispY);

#ifdef __cplusplus
}
#endif

#endif /* TS_CALIBRATION_H */
