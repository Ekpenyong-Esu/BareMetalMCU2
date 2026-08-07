/**
  ******************************************************************************
  * @file    ultrasonic_measure.h
  * @brief   Ultrasonic measurement cycle
  ******************************************************************************
  */

#ifndef ULTRASONIC_MEASURE_H
#define ULTRASONIC_MEASURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ultrasonic_types.h"

/**
 * @brief   Emit the trigger pulse and arm the echo capture.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_StartMeasurement(ULTRASONIC_Handle_t *hultra);

/**
 * @brief   Abandon a measurement in flight and return the sensor to idle.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_AbortMeasurement(ULTRASONIC_Handle_t *hultra);

bool ULTRASONIC_IsMeasurementComplete(const ULTRASONIC_Handle_t *hultra);

/**
 * @brief   Block until the echo has been captured.
 * @note    Aborts the cycle on timeout, so the sensor does not stay busy.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_WaitForMeasurement(ULTRASONIC_Handle_t *hultra,
                                                       uint32_t timeout);

/**
 * @brief   Full measurement cycle, reporting why it failed.
 * @param   distance Optional, receives the distance in mm.
 */
ULTRASONIC_StatusTypeDef ULTRASONIC_Measure(ULTRASONIC_Handle_t *hultra, uint16_t *distance);

/**
 * @brief   Full measurement cycle.
 * @retval  Distance in mm, 0 on any failure.
 */
uint16_t ULTRASONIC_MeasureDistance(ULTRASONIC_Handle_t *hultra);

/**
 * @brief   Last in-range distance in mm, 0 if the last measurement failed.
 */
uint16_t ULTRASONIC_GetDistance(const ULTRASONIC_Handle_t *hultra);

/**
 * @brief   Timer input capture callback, to be called from the timer ISR.
 */
void ULTRASONIC_TIM_IC_CaptureCallback(ULTRASONIC_Handle_t *hultra);

#ifdef __cplusplus
}
#endif

#endif /* ULTRASONIC_MEASURE_H */
