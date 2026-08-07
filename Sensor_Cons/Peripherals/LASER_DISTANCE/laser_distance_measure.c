/**
  ******************************************************************************
  * @file    laser_distance_measure.c
  * @brief   Laser distance measurement control and result access
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "laser_distance_measure.h"
#include "laser_distance_i2c.h"
#include "laser_distance_vl53l0x.h"
#include "log.h"

/* Public functions ----------------------------------------------------------*/

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_StartContinuous(LASER_DISTANCE_Handle_t *hlaser)
{
    LASER_DISTANCE_StatusTypeDef status;

    if (hlaser == NULL || !hlaser->isInitialized)
    {
        return LASER_DISTANCE_NOT_INITIALIZED;
    }

    switch (hlaser->config.sensorType)
    {
        case LASER_DISTANCE_VL53L0X:
            status = LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_SYSRANGE_START,
                                                 VL53L0X_SYSRANGE_BACK_TO_BACK);
            break;
        case LASER_DISTANCE_TFMINI:
            /* TFmini is always in continuous mode */
            status = LASER_DISTANCE_OK;
            break;
        default:
            return LASER_DISTANCE_ERROR;
    }

    if (status == LASER_DISTANCE_OK)
    {
        /* StopContinuous clears this flag for every sensor, so every sensor
           has to set it, not just the TFmini. */
        hlaser->isMeasuring = true;
    }

    return status;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_StopContinuous(LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL)
    {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    hlaser->isMeasuring = false;

    switch (hlaser->config.sensorType)
    {
        case LASER_DISTANCE_VL53L0X:
            return LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_SYSRANGE_START,
                                               VL53L0X_SYSRANGE_SINGLE_SHOT);
        default:
            return LASER_DISTANCE_OK;
    }
}

uint16_t LASER_DISTANCE_MeasureDistance(LASER_DISTANCE_Handle_t *hlaser)
{
    LASER_DISTANCE_StatusTypeDef status;

    if (hlaser == NULL || !hlaser->isInitialized)
    {
        log_error("LASER_DISTANCE: Sensor not initialized");
        return 0;
    }

    log_debug("LASER_DISTANCE: Starting distance measurement, sensor type: %d",
              hlaser->config.sensorType);

    switch (hlaser->config.sensorType)
    {
        case LASER_DISTANCE_VL53L0X:
            status = LASER_DISTANCE_VL53L0X_ReadRange(hlaser);
            break;
        default:
            log_error("LASER_DISTANCE: Unsupported sensor type %d", hlaser->config.sensorType);
            return 0;
    }

    if (status != LASER_DISTANCE_OK)
    {
        log_warning("LASER_DISTANCE: Measurement failed with status: %d", status);
        return 0;
    }

    hlaser->lastMeasurement.timestamp = HAL_GetTick();
    log_debug("LASER_DISTANCE: Measurement complete - Distance: %u mm, Status: %u",
              (unsigned)hlaser->lastMeasurement.distance,
              (unsigned)hlaser->lastMeasurement.rangeStatus);

    return hlaser->lastMeasurement.distance;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_GetMeasurement(const LASER_DISTANCE_Handle_t *hlaser,
                                                           LASER_DISTANCE_Measurement_t *measurement)
{
    if (hlaser == NULL || measurement == NULL)
    {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    *measurement = hlaser->lastMeasurement;

    return LASER_DISTANCE_OK;
}

uint16_t LASER_DISTANCE_GetDistance(const LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL)
    {
        return 0;
    }

    return hlaser->lastMeasurement.distance;
}

bool LASER_DISTANCE_IsMeasurementReady(const LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL || !hlaser->isInitialized)
    {
        return false;
    }

    switch (hlaser->config.sensorType)
    {
        case LASER_DISTANCE_VL53L0X:
            return LASER_DISTANCE_VL53L0X_IsReady(hlaser);
        case LASER_DISTANCE_TFMINI:
            /* TFmini always has data available in continuous mode */
            return hlaser->isMeasuring;
        default:
            return false;
    }
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_ChangeAddress(LASER_DISTANCE_Handle_t *hlaser,
                                                          uint8_t newAddress)
{
    const uint8_t address7bit = (uint8_t)(newAddress >> 1);

    if (hlaser == NULL || !hlaser->isInitialized)
    {
        return LASER_DISTANCE_NOT_INITIALIZED;
    }

    if (hlaser->config.sensorType != LASER_DISTANCE_VL53L0X)
    {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    /* An out-of-range address would leave the device unreachable. */
    if ((newAddress & 0x01U) != 0U ||
        address7bit < LASER_DISTANCE_MIN_7BIT_ADDRESS ||
        address7bit > LASER_DISTANCE_MAX_7BIT_ADDRESS)
    {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    return LASER_DISTANCE_VL53L0X_SetAddress(hlaser, newAddress);
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Calibrate(LASER_DISTANCE_Handle_t *hlaser)
{
    if (hlaser == NULL || !hlaser->isInitialized)
    {
        return LASER_DISTANCE_NOT_INITIALIZED;
    }

    if (hlaser->config.sensorType != LASER_DISTANCE_VL53L0X)
    {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    return LASER_DISTANCE_VL53L0X_Calibrate(hlaser);
}
