/**
 ******************************************************************************
 * @file    joystick_adc.h
 * @brief   Joystick hardware access - internal to the JOYSTICK driver
 * @details Owns the two ADC channel reads and the push-switch GPIO.
 *          Not part of joystick.h.
 ******************************************************************************
 */

#ifndef JOYSTICK_ADC_H
#define JOYSTICK_ADC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "joystick_types.h"

/**
 * @brief   Check the wiring and bring up the push-switch pin.
 * @param   pins   Wiring to validate and initialise.
 * @param   config Supplies the switch polarity, which sets the pull direction.
 * @retval  JOYSTICK_OK or JOYSTICK_INVALID_PARAM / JOYSTICK_ERROR
 * @note    The ADC itself is the caller's; only the button pin is touched here.
 */
JOYSTICK_StatusTypeDef JOYSTICK_ADC_Init(const JOYSTICK_Pins_t *pins,
                                         const JOYSTICK_Config_t *config);

/**
 * @brief   Release the push-switch pin.
 * @param   pins Wiring to release.
 */
void JOYSTICK_ADC_DeInit(const JOYSTICK_Pins_t *pins);

/**
 * @brief   Read one axis.
 * @param   pins    Wiring supplying the ADC handle.
 * @param   channel ADC channel to sample.
 * @param   raw     Receives the raw count.
 * @retval  JOYSTICK_OK or JOYSTICK_ERROR
 */
JOYSTICK_StatusTypeDef JOYSTICK_ADC_ReadAxis(const JOYSTICK_Pins_t *pins, uint32_t channel,
                                             uint16_t *raw);

/**
 * @brief   Read the push switch.
 * @param   pins       Wiring supplying the button pin.
 * @param   activeLow  True when the switch pulls the pin to ground.
 * @return  true while pressed; false when no button is wired.
 */
bool JOYSTICK_ADC_ReadButton(const JOYSTICK_Pins_t *pins, bool activeLow);

#ifdef __cplusplus
}
#endif

#endif /* JOYSTICK_ADC_H */
