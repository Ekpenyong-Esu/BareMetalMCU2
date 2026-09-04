/**
 ******************************************************************************
 * @file    servo_calibrate.h
 * @brief   Servo travel-limit calibration
 ******************************************************************************
 */

#ifndef SERVO_CALIBRATE_H
#define SERVO_CALIBRATE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "servo_types.h"

/**
 * @brief   Narrow the travel range from below.
 * @note    Rejected if it would invert or empty the range.
 */
SERVO_StatusTypeDef SERVO_CalibrateMin(SERVO_Handle_t *hservo, uint16_t angle);

/**
 * @brief   Narrow the travel range from above.
 * @note    Rejected if it would invert or empty the range.
 */
SERVO_StatusTypeDef SERVO_CalibrateMax(SERVO_Handle_t *hservo, uint16_t angle);

/**
 * @brief   Restore the factory travel range and move back inside it.
 */
SERVO_StatusTypeDef SERVO_ResetCalibration(SERVO_Handle_t *hservo);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CALIBRATE_H */
