/**
  ******************************************************************************
  * @file    ultrasonic_convert.h
  * @brief   Echo width to distance conversion
  ******************************************************************************
  */

#ifndef ULTRASONIC_CONVERT_H
#define ULTRASONIC_CONVERT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ultrasonic_types.h"

/**
 * @brief   Speed of sound in air, in mm/s, corrected for temperature.
 */
uint32_t ULTRASONIC_SpeedOfSound(int8_t temperature);

/**
 * @brief   Convert an echo width in microseconds to a one-way distance in mm.
 * @note    Integer only: this runs in interrupt context, where float math would
 *          force the FPU context to be stacked.
 */
uint16_t ULTRASONIC_EchoToDistance(uint32_t echoUs, int8_t temperature);

/**
 * @brief   Echo width in ticks between two capture values, across one wrap.
 */
uint32_t ULTRASONIC_EchoWidth(uint32_t start, uint32_t end, uint32_t period);

bool ULTRASONIC_IsValidDistance(const ULTRASONIC_Handle_t *hultra, uint16_t distance);

#ifdef __cplusplus
}
#endif

#endif /* ULTRASONIC_CONVERT_H */
