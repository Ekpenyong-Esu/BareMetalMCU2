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
 * @brief   Pulse width in microseconds for @p angle, clamped to the config range.
 */
uint16_t SERVO_AngleToPulseWidth(uint16_t angle, const SERVO_Config_t *config);

/**
 * @brief   Angle in degrees for @p pulseWidth, clamped to the config range.
 */
uint16_t SERVO_PulseWidthToAngle(uint16_t pulseWidth, const SERVO_Config_t *config);

bool SERVO_IsValidAngle(uint16_t angle, const SERVO_Config_t *config);

bool SERVO_IsValidPulseWidth(uint16_t pulseWidth, const SERVO_Config_t *config);

/**
 * @brief   Reject a config whose ranges are empty, inverted or inconsistent.
 */
SERVO_StatusTypeDef SERVO_ValidateConfig(const SERVO_Config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CONVERT_H */
