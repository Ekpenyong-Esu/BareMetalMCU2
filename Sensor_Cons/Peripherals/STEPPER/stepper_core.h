/**
 ******************************************************************************
 * @file    stepper_core.h
 * @brief   Stepper motor lifecycle and configuration
 ******************************************************************************
 */

#ifndef STEPPER_CORE_H
#define STEPPER_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stepper_types.h"

/**
 * @brief   Configure the coil pins and the timer used as the step time base.
 * @param   htim Timer handle whose Instance the caller has already selected;
 *          this driver reprograms it as a free-running 1 MHz counter.
 * @retval  STEPPER_INVALID_PARAM if any argument or coil port is NULL.
 */
STEPPER_StatusTypeDef STEPPER_Init(STEPPER_Handle_t *hstep, TIM_HandleTypeDef *htim,
                                   const STEPPER_Pins_t *pins);

/**
 * @brief   Stop the motor, release the coils and the time base.
 */
STEPPER_StatusTypeDef STEPPER_DeInit(STEPPER_Handle_t *hstep);

/**
 * @brief   Apply a motor configuration.
 * @note    Rejected configurations leave the previous one in place.
 */
STEPPER_StatusTypeDef STEPPER_Config(STEPPER_Handle_t *hstep, const STEPPER_Config_t *config);

/**
 * @brief   Configuration for a generic 200 step/rev motor in full-step mode.
 */
STEPPER_Config_t STEPPER_GetDefaultConfig(void);

/**
 * @brief   Pin assignment used by the STEPPER example wiring.
 */
STEPPER_Pins_t STEPPER_GetDefaultPins(void);

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_CORE_H */
