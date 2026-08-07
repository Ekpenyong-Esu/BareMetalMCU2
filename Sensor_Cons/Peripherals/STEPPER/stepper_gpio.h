/**
  ******************************************************************************
  * @file    stepper_gpio.h
  * @brief   Coil pin configuration and drive
  * @note    Internal to the driver; not part of stepper.h.
  ******************************************************************************
  */

#ifndef STEPPER_GPIO_H
#define STEPPER_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stepper_types.h"

/**
 * @brief Configure the four coil pins as push-pull outputs, all de-energised.
 * @retval STEPPER_INVALID_PARAM if any port is NULL.
 */
STEPPER_StatusTypeDef STEPPER_GPIO_Init(const STEPPER_Pins_t *pins);

/**
 * @brief Return the coil pins to their reset state.
 */
void STEPPER_GPIO_DeInit(const STEPPER_Pins_t *pins);

/**
 * @brief Drive the coils from one sequence entry.
 * @param pattern Array of STEPPER_COIL_COUNT flags.
 */
void STEPPER_GPIO_ApplyPattern(const STEPPER_Pins_t *pins, const uint8_t *pattern);

/**
 * @brief De-energise all coils.
 */
void STEPPER_GPIO_ReleaseCoils(const STEPPER_Pins_t *pins);

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_GPIO_H */
