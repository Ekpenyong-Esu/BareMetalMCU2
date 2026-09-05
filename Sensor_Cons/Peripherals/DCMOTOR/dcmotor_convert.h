/**
 ******************************************************************************
 * @file    dcmotor_convert.h
 * @brief   Speed percent <-> duty conversion and range validation
 ******************************************************************************
 */

#ifndef DCMOTOR_CONVERT_H
#define DCMOTOR_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dcmotor_types.h"

/**
 * @brief   Compare value for a speed percentage, clamped to the resolution.
 * @param   percent    Speed in percent (0..100).
 * @param   speedSteps Duty resolution from the config.
 * @return  Compare value in 0..speedSteps.
 */
uint32_t DCMOTOR_PercentToDuty(uint8_t percent, uint16_t speedSteps);

/**
 * @brief   Speed percentage for a compare value, clamped to 0..100.
 * @param   duty       Compare value.
 * @param   speedSteps Duty resolution from the config.
 * @return  Speed in percent.
 */
uint8_t DCMOTOR_DutyToPercent(uint32_t duty, uint16_t speedSteps);

/**
 * @brief   Check a speed against the 0..100 percent range.
 * @param   percent Speed in percent.
 * @return  true if within range.
 */
bool DCMOTOR_IsValidSpeed(uint8_t percent);

/**
 * @brief   Reject a config whose carrier or resolution is outside the driver limits.
 * @param   config Config to validate.
 * @retval  DCMOTOR_OK or DCMOTOR_INVALID_PARAM
 */
DCMOTOR_StatusTypeDef DCMOTOR_ValidateConfig(const DCMOTOR_Config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* DCMOTOR_CONVERT_H */
