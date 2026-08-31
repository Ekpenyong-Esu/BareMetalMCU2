/**
  ******************************************************************************
  * @file    laser_distance_i2c.c
  * @brief   Register-level I2C transport for the laser distance sensors
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "laser_distance_i2c.h"
#include "i2c.h"

/* Private variables ---------------------------------------------------------*/

/* The sensor's address is re-programmable, so the device record follows
   whatever the handle currently believes the sensor answers to. */
static I2C_Device_t s_device;

/* Private functions ---------------------------------------------------------*/

/**
 * @brief The bus device record, pointed at the sensor @p hlaser describes
 */
static I2C_Device_t *LASER_DISTANCE_I2C_Device(const LASER_DISTANCE_Handle_t *hlaser)
{
    const I2C_ConfigTypeDef config = I2C_ConfigDefault();

    I2C_DeviceInit(&s_device, hlaser->config.i2cAddress, &config);

    return &s_device;
}

/* Public functions ----------------------------------------------------------*/

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_WriteReg(const LASER_DISTANCE_Handle_t *hlaser,
                                                         uint8_t reg, uint8_t value)
{
    if (I2C_Mem_Write(LASER_DISTANCE_I2C_Device(hlaser), reg, I2C_MEMADD_SIZE_8BIT,
                      &value, 1, LASER_DISTANCE_I2C_TIMEOUT) == I2C_OK)
    {
        return LASER_DISTANCE_OK;
    }

    return LASER_DISTANCE_I2C_ERROR;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadReg(const LASER_DISTANCE_Handle_t *hlaser,
                                                        uint8_t reg, uint8_t *value)
{
    if (I2C_Mem_Read(LASER_DISTANCE_I2C_Device(hlaser), reg, I2C_MEMADD_SIZE_8BIT,
                     value, 1, LASER_DISTANCE_I2C_TIMEOUT) == I2C_OK)
    {
        return LASER_DISTANCE_OK;
    }

    return LASER_DISTANCE_I2C_ERROR;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_I2C_ReadMulti(const LASER_DISTANCE_Handle_t *hlaser,
                                                          uint8_t reg, uint8_t *buffer,
                                                          uint16_t length)
{
    if (I2C_Mem_Read(LASER_DISTANCE_I2C_Device(hlaser), reg, I2C_MEMADD_SIZE_8BIT,
                     buffer, length, LASER_DISTANCE_I2C_TIMEOUT) == I2C_OK)
    {
        return LASER_DISTANCE_OK;
    }

    return LASER_DISTANCE_I2C_ERROR;
}
