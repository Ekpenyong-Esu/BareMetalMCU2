/**
 ******************************************************************************
 * @file    dcmotor_core.h
 * @brief   DC motor lifecycle and configuration
 ******************************************************************************
 */

#ifndef DCMOTOR_CORE_H
#define DCMOTOR_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dcmotor_types.h"

/**
 * @brief   Bring up the bridge pins and PWM base, leaving the motor stopped.
 * @param   hmotor Uninitialised handle to populate (zeroed on entry).
 * @param   pins   Timer, channel and the three bridge pins; caller must have
 *                 set pins->htim->Instance before the call.
 * @retval  DCMOTOR_OK or DCMOTOR_INVALID_PARAM / DCMOTOR_ERROR
 * @note    Uses DCMOTOR_GetDefaultConfig; call DCMOTOR_Config to change it.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Init(DCMOTOR_Handle_t *hmotor, const DCMOTOR_Pins_t *pins);

/**
 * @brief   Stop the motor, release the bridge and mark the handle uninitialised.
 * @param   hmotor Initialised handle.
 * @retval  DCMOTOR_OK or DCMOTOR_NOT_INITIALIZED / DCMOTOR_INVALID_PARAM
 */
DCMOTOR_StatusTypeDef DCMOTOR_DeInit(DCMOTOR_Handle_t *hmotor);

/**
 * @brief   Reprogram the carrier and duty resolution, stopping the motor first.
 * @param   hmotor Initialised handle.
 * @param   config New settings; must pass DCMOTOR_ValidateConfig.
 * @retval  DCMOTOR_OK or DCMOTOR_INVALID_PARAM / DCMOTOR_ERROR
 * @note    A rejected config leaves the previous one in place.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Config(DCMOTOR_Handle_t *hmotor, const DCMOTOR_Config_t *config);

/**
 * @brief   Factory settings: 20 kHz carrier, 1000 duty steps, coast on stop.
 * @return  Default config.
 */
DCMOTOR_Config_t DCMOTOR_GetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* DCMOTOR_CORE_H */
