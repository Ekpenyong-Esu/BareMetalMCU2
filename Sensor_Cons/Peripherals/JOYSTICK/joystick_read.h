/**
 ******************************************************************************
 * @file    joystick_read.h
 * @brief   Joystick sampling - what the application calls in its loop
 * @details Reports position only. Mapping a position onto an LED, an H-bridge,
 *          a stepper or a servo is the application's decision, so nothing here
 *          knows those drivers exist.
 ******************************************************************************
 */

#ifndef JOYSTICK_READ_H
#define JOYSTICK_READ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "joystick_types.h"

/**
 * @brief   Sample both axes and the button in one go.
 * @param   hjoy     Initialised handle.
 * @param   position Receives the deflection of both axes and the button state.
 * @retval  JOYSTICK_OK on success
 */
JOYSTICK_StatusTypeDef JOYSTICK_Read(const JOYSTICK_Handle_t *hjoy, JOYSTICK_Position_t *position);

/**
 * @brief   Sample the X axis alone.
 * @param   hjoy    Initialised handle.
 * @param   percent Receives -100 (left) to +100 (right).
 * @retval  JOYSTICK_OK on success
 */
JOYSTICK_StatusTypeDef JOYSTICK_ReadX(const JOYSTICK_Handle_t *hjoy, int8_t *percent);

/**
 * @brief   Sample the Y axis alone.
 * @param   hjoy    Initialised handle.
 * @param   percent Receives -100 (down) to +100 (up).
 * @retval  JOYSTICK_OK on success
 */
JOYSTICK_StatusTypeDef JOYSTICK_ReadY(const JOYSTICK_Handle_t *hjoy, int8_t *percent);

/**
 * @brief   Sample both axes and reduce them to one direction.
 * @param   hjoy      Initialised handle.
 * @param   direction Receives the dominant direction.
 * @retval  JOYSTICK_OK on success
 */
JOYSTICK_StatusTypeDef JOYSTICK_ReadDirection(const JOYSTICK_Handle_t *hjoy,
                                              JOYSTICK_Direction_t *direction);

/**
 * @brief   Read the push switch.
 * @param   hjoy Initialised handle.
 * @return  true while pressed; false if uninitialised or no button is wired.
 */
bool JOYSTICK_IsPressed(const JOYSTICK_Handle_t *hjoy);

/**
 * @brief   Test whether both axes are inside the deadzone.
 * @param   hjoy Initialised handle.
 * @return  true when the stick is resting.
 */
bool JOYSTICK_IsCentered(const JOYSTICK_Handle_t *hjoy);

#ifdef __cplusplus
}
#endif

#endif /* JOYSTICK_READ_H */
