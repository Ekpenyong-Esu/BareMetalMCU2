/**
 ******************************************************************************
 * @file    joystick.h
 * @brief   Analog two-axis joystick driver - public aggregator
 * @details Include this single header for the full driver API. For the
 *          low-level ADC and button transport include joystick_adc.h directly.
 *
 *          The driver reports position only: each axis as -100..+100 with a
 *          deadzone applied, plus the push switch. What that position means is
 *          the application's decision, so this driver does not depend on LED,
 *          DCMOTOR, STEPPER or SERVO.
 *
 *          Typical mappings:
 *          - LED PWM     : JOYSTICK_Magnitude(y) -> duty percent
 *          - L298N motor : sign of y -> direction, magnitude -> speed percent
 *          - Stepper     : sign of x -> direction, magnitude -> step rate
 *          - Servo       : x scaled from -100..+100 onto min..max angle
 ******************************************************************************
 */

#ifndef JOYSTICK_H
#define JOYSTICK_H

#include "joystick_types.h"
#include "joystick_core.h"
#include "joystick_read.h"
#include "joystick_convert.h"

#endif /* JOYSTICK_H */
