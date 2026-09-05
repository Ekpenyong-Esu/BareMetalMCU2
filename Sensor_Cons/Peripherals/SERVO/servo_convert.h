/**
 ******************************************************************************
 * @file    servo_convert.h
 * @brief   Angle <-> pulse width conversion and range validation
 ******************************************************************************
 */

#ifndef SERVO_CONVERT_H
#define SERVO_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "servo_types.h"

/**
 * @brief   Pulse width for angle, clamped to config range.
 * @param   angle  Angle in degrees.
 * @param   config Limits or NULL (uses factory 500-2500 us).
 * @return  Pulse width in microseconds.
 */
uint16_t SERVO_AngleToPulseWidth(uint16_t angle, const SERVO_Config_t *config);

/**
 * @brief   Angle for pulse width, clamped to config range.
 * @param   pulseWidth Pulse width in microseconds.
 * @param   config     Limits or NULL (uses factory 0-180°).
 * @return  Angle in degrees.
 */
uint16_t SERVO_PulseWidthToAngle(uint16_t pulseWidth, const SERVO_Config_t *config);

/**
 * @brief   Check angle against config limits.
 * @param   angle  Angle in degrees.
 * @param   config Limits or NULL (uses factory 0-180°).
 * @return  true if within range.
 */
bool SERVO_IsValidAngle(uint16_t angle, const SERVO_Config_t *config);

/**
 * @brief   Check pulse width against config limits.
 * @param   pulseWidth Pulse width in microseconds.
 * @param   config     Limits or NULL (uses factory 500-2500 us).
 * @return  true if within range.
 */
bool SERVO_IsValidPulseWidth(uint16_t pulseWidth, const SERVO_Config_t *config);

/**
 * @brief   Reject a config whose ranges are empty, inverted or inconsistent.
 * @param   config Config to validate.
 * @retval  SERVO_OK or SERVO_INVALID_PARAM
 */
SERVO_StatusTypeDef SERVO_ValidateConfig(const SERVO_Config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CONVERT_H */
