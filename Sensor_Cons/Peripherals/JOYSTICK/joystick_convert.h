/**
 ******************************************************************************
 * @file    joystick_convert.h
 * @brief   Joystick scaling maths - no hardware access
 ******************************************************************************
 */

#ifndef JOYSTICK_CONVERT_H
#define JOYSTICK_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "joystick_types.h"

/**
 * @brief   Turn a raw ADC count into signed deflection.
 * @param   raw             Raw ADC count.
 * @param   center          Raw count with the stick at rest.
 * @param   rawMax          ADC full-scale count.
 * @param   deadzonePercent Band around centre reported as zero.
 * @param   invert          Flip the sign to match how the module is mounted.
 * @return  -100..+100, where 0 means resting.
 * @note    Each side of centre is scaled against its own span, so an off-centre
 *          rest position still reaches full travel both ways.
 */
int8_t JOYSTICK_RawToPercent(uint16_t raw, uint16_t center, uint16_t rawMax,
                             uint8_t deadzonePercent, bool invert);

/**
 * @brief   Reduce two axes to a single direction.
 * @param   xPercent         X deflection.
 * @param   yPercent         Y deflection.
 * @param   thresholdPercent Deflection needed before a direction is named.
 * @return  The dominant direction, or JOYSTICK_DIR_CENTER below the threshold.
 */
JOYSTICK_Direction_t JOYSTICK_PercentToDirection(int8_t xPercent, int8_t yPercent,
                                                 uint8_t thresholdPercent);

/**
 * @brief   Deflection without its sign, ready to use as a speed or brightness.
 * @param   percent Signed deflection.
 * @return  0..100.
 */
uint8_t JOYSTICK_Magnitude(int8_t percent);

/**
 * @brief   Check a configuration before it is stored.
 * @param   config Candidate configuration.
 * @retval  JOYSTICK_OK or JOYSTICK_INVALID_PARAM
 */
JOYSTICK_StatusTypeDef JOYSTICK_ValidateConfig(const JOYSTICK_Config_t *config);

#ifdef __cplusplus
}
#endif

#endif /* JOYSTICK_CONVERT_H */
