/**
 ******************************************************************************
 * @file    joystick_core.h
 * @brief   Joystick lifecycle - bring-up, configuration and calibration
 ******************************************************************************
 */

#ifndef JOYSTICK_CORE_H
#define JOYSTICK_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "joystick_types.h"

/**
 * @brief   Sensible starting configuration.
 * @return  12-bit scale, 10% deadzone, neither axis inverted, active-low button.
 */
JOYSTICK_Config_t JOYSTICK_GetDefaultConfig(void);

/**
 * @brief   Prepare a joystick for reading.
 * @param   hjoy Handle to fill in.
 * @param   pins Wiring; the ADC handle inside must already be initialised.
 * @retval  JOYSTICK_OK on success
 * @note    Centre is assumed to be mid-scale until JOYSTICK_Calibrate measures it.
 */
JOYSTICK_StatusTypeDef JOYSTICK_Init(JOYSTICK_Handle_t *hjoy, const JOYSTICK_Pins_t *pins);

/**
 * @brief   Release the joystick.
 * @param   hjoy Handle to release.
 * @retval  JOYSTICK_OK on success
 */
JOYSTICK_StatusTypeDef JOYSTICK_DeInit(JOYSTICK_Handle_t *hjoy);

/**
 * @brief   Replace the scaling and orientation settings.
 * @param   hjoy   Initialised handle.
 * @param   config New configuration.
 * @retval  JOYSTICK_OK on success, JOYSTICK_INVALID_PARAM if the config is rejected
 */
JOYSTICK_StatusTypeDef JOYSTICK_Config(JOYSTICK_Handle_t *hjoy, const JOYSTICK_Config_t *config);

/**
 * @brief   Record where the stick rests and treat that as zero.
 * @param   hjoy Initialised handle.
 * @retval  JOYSTICK_OK on success
 * @note    Leave the stick untouched while this runs; it averages several readings.
 */
JOYSTICK_StatusTypeDef JOYSTICK_Calibrate(JOYSTICK_Handle_t *hjoy);

#ifdef __cplusplus
}
#endif

#endif /* JOYSTICK_CORE_H */
