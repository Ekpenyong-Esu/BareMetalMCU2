/**
  ******************************************************************************
  * @file    laser_distance_i2c.h
  * @brief   Register-level I2C transport for the laser distance sensors
  * @details Internal to the driver: the only module that talks to the I2C
  *          peripheral. Not part of the public aggregator.
  ******************************************************************************
  */

#ifndef __LASER_DISTANCE_I2C_H__
#define __LASER_DISTANCE_I2C_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "laser_distance_types.h"

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Write a single sensor register
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   reg Register address
 * @param   value Value to write
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_WriteReg(const LASER_DISTANCE_Handle_t *hlaser,
                                                         uint8_t reg, uint8_t value);

/**
 * @brief   Read a single sensor register
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   reg Register address
 * @param   value Pointer to store the value read
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadReg(const LASER_DISTANCE_Handle_t *hlaser,
                                                        uint8_t reg, uint8_t *value);

/**
 * @brief   Read a block of consecutive sensor registers
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   reg Starting register address
 * @param   buffer Destination buffer
 * @param   length Number of bytes to read
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadMulti(const LASER_DISTANCE_Handle_t *hlaser,
                                                          uint8_t reg, uint8_t *buffer,
                                                          uint16_t length);

#ifdef __cplusplus
}
#endif

#endif /* __LASER_DISTANCE_I2C_H__ */
