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
 * @brief   Narrow travel range from below.
 * @param   hservo Initialised handle.
 * @param   angle  New minimum angle; must be < maxAngle.
 * @retval  SERVO_OK or SERVO_INVALID_PARAM / handle error
 * @note    Rejected if it would invert or empty the range.
 */
SERVO_StatusTypeDef SERVO_CalibrateMin(SERVO_Handle_t *hservo, uint16_t angle);

/**
 * @brief   Narrow travel range from above.
 * @param   hservo Initialised handle.
 * @param   angle  New maximum angle; must be > minAngle.
 * @retval  SERVO_OK or SERVO_INVALID_PARAM / handle error
 * @note    Rejected if it would invert or empty the range.
 */
SERVO_StatusTypeDef SERVO_CalibrateMax(SERVO_Handle_t *hservo, uint16_t angle);

/**
 * @brief   Restore factory travel range and move back inside it.
 * @param   hservo Initialised handle.
 * @retval  SERVO_OK or handle error
 */
SERVO_StatusTypeDef SERVO_ResetCalibration(SERVO_Handle_t *hservo);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CALIBRATE_H */
