/**
 ******************************************************************************
 * @file    laser_distance_i2c.c
 * @brief   Register-level I2C transport for the laser distance sensors
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "laser_distance_i2c.h"
#include "i2c.h"

/* Public functions ----------------------------------------------------------*/

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_WriteReg(LASER_DISTANCE_Handle_t *hlaser,
                                                         uint8_t reg, uint8_t value) {
    if (I2C_Mem_Write(&hlaser->device, reg, I2C_MEMADD_SIZE_8BIT, &value, 1,
                      LASER_DISTANCE_I2C_TIMEOUT) == I2C_OK) {
        return LASER_DISTANCE_OK;
    }

    return LASER_DISTANCE_I2C_ERROR;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadReg(LASER_DISTANCE_Handle_t *hlaser,
                                                        uint8_t reg, uint8_t *value) {
    if (I2C_Mem_Read(&hlaser->device, reg, I2C_MEMADD_SIZE_8BIT, value, 1,
                     LASER_DISTANCE_I2C_TIMEOUT) == I2C_OK) {
        return LASER_DISTANCE_OK;
    }

    return LASER_DISTANCE_I2C_ERROR;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadMulti(LASER_DISTANCE_Handle_t *hlaser,
                                                          uint8_t reg, uint8_t *buffer,
                                                          uint16_t length) {
    if (I2C_Mem_Read(&hlaser->device, reg, I2C_MEMADD_SIZE_8BIT, buffer, length,
                     LASER_DISTANCE_I2C_TIMEOUT) == I2C_OK) {
        return LASER_DISTANCE_OK;
    }

    return LASER_DISTANCE_I2C_ERROR;
}
