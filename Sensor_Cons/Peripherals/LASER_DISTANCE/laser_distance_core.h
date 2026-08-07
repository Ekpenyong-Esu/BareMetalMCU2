/**
  ******************************************************************************
  * @file    laser_distance_core.h
  * @brief   Laser distance sensor lifecycle and configuration
  ******************************************************************************
  */

#ifndef __LASER_DISTANCE_CORE_H__
#define __LASER_DISTANCE_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "laser_distance_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Initialize laser distance sensor
 * @details Applies the default configuration for the sensor type and runs the
 *          sensor specific start-up sequence
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   hi2c Pointer to I2C handle
 * @param   sensorType Type of laser distance sensor
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Init(LASER_DISTANCE_Handle_t *hlaser,
                                                 I2C_HandleTypeDef *hi2c,
                                                 LASER_DISTANCE_SensorType_t sensorType);

/**
 * @brief   Deinitialize laser distance sensor
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_DeInit(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Configure laser distance sensor parameters
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   config Pointer to configuration structure
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Config(LASER_DISTANCE_Handle_t *hlaser,
                                                   const LASER_DISTANCE_Config_t *config);

/**
 * @brief   Get default configuration for sensor type
 * @param   sensorType Type of laser distance sensor
 * @retval  LASER_DISTANCE_Config_t Default configuration
 */
LASER_DISTANCE_Config_t LASER_DISTANCE_GetDefaultConfig(LASER_DISTANCE_SensorType_t sensorType);

/**
 * @brief   Check if distance is within sensor range
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   distance Distance to validate in mm
 * @retval  bool True if distance is valid
 */
bool LASER_DISTANCE_IsValidDistance(const LASER_DISTANCE_Handle_t *hlaser, uint16_t distance);

/**
 * @brief   Get sensor status string
 * @param   status Status code
 * @retval  const char* Status description string
 */
const char* LASER_DISTANCE_GetStatusString(LASER_DISTANCE_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* __LASER_DISTANCE_CORE_H__ */
