/**
  ******************************************************************************
  * @file    laser_distance_measure.h
  * @brief   Laser distance measurement control and result access
  ******************************************************************************
  */

#ifndef __LASER_DISTANCE_MEASURE_H__
#define __LASER_DISTANCE_MEASURE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "laser_distance_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Start continuous measurement mode
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_StartContinuous(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Stop continuous measurement mode
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_StopContinuous(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Perform single distance measurement
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if measurement failed)
 */
uint16_t LASER_DISTANCE_MeasureDistance(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Get last measurement data
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   measurement Pointer to measurement data structure
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_GetMeasurement(const LASER_DISTANCE_Handle_t *hlaser,
                                                           LASER_DISTANCE_Measurement_t *measurement);

/**
 * @brief   Get last measured distance
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  uint16_t Distance in millimeters (0 if no valid measurement)
 */
uint16_t LASER_DISTANCE_GetDistance(const LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Check if new measurement is available
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  bool True if measurement is ready
 */
bool LASER_DISTANCE_IsMeasurementReady(const LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Change I2C address (VL53L0X only)
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   newAddress New address in 8-bit (shifted) form
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_ChangeAddress(LASER_DISTANCE_Handle_t *hlaser,
                                                          uint8_t newAddress);

/**
 * @brief   Perform sensor calibration (VL53L0X only)
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Calibrate(LASER_DISTANCE_Handle_t *hlaser);

#ifdef __cplusplus
}
#endif

#endif /* __LASER_DISTANCE_MEASURE_H__ */
