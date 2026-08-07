/**
  ******************************************************************************
  * @file    laser_distance_vl53l0x.c
  * @brief   VL53L0X specific operations
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "laser_distance_vl53l0x.h"
#include "laser_distance_i2c.h"
#include "log.h"

/* Private defines -----------------------------------------------------------*/

/** @brief Offsets inside the RESULT_RANGE_STATUS block */
#define VL53L0X_RANGE_OFFSET_STATUS     0U
#define VL53L0X_RANGE_OFFSET_SIGNAL     6U
#define VL53L0X_RANGE_OFFSET_AMBIENT    8U
#define VL53L0X_RANGE_OFFSET_DISTANCE   10U

/** @brief Poll interval while waiting for a range result, in ms */
#define VL53L0X_POLL_INTERVAL_MS        1U

/* Private variables ---------------------------------------------------------*/

/**
 * @brief Register writes applied at start-up
 * @note  Simplified: a full ST initialisation sequence also tunes SPADs and
 *        the reference calibration.
 */
static const struct {
    uint8_t reg;
    uint8_t value;
} vl53l0xInitSequence[] = {
    { VL53L0X_REG_READOUT_AVERAGING_SAMPLE_PERIOD,  VL53L0X_READOUT_AVERAGING_PERIOD },
    { VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD,    VL53L0X_VCSEL_PERIOD_PRE_RANGE },
    { VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD,  VL53L0X_VCSEL_PERIOD_FINAL_RANGE },
};

/* Public functions ----------------------------------------------------------*/

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_Init(LASER_DISTANCE_Handle_t *hlaser)
{
    uint8_t deviceId;

    log_debug("LASER_DISTANCE: Initializing VL53L0X sensor");

    if (LASER_DISTANCE_I2C_ReadReg(hlaser, VL53L0X_REG_IDENTIFICATION_MODEL_ID,
                                   &deviceId) != LASER_DISTANCE_OK)
    {
        log_error("LASER_DISTANCE: Failed to read VL53L0X device ID");
        return LASER_DISTANCE_I2C_ERROR;
    }

    log_debug("LASER_DISTANCE: VL53L0X device ID: 0x%02X", deviceId);

    if (deviceId != VL53L0X_EXPECTED_DEVICE_ID)
    {
        log_error("LASER_DISTANCE: Invalid VL53L0X device ID: 0x%02X, expected: 0x%02X",
                  deviceId, VL53L0X_EXPECTED_DEVICE_ID);
        return LASER_DISTANCE_ERROR;
    }

    for (size_t i = 0; i < (sizeof(vl53l0xInitSequence) / sizeof(vl53l0xInitSequence[0])); i++)
    {
        if (LASER_DISTANCE_I2C_WriteReg(hlaser, vl53l0xInitSequence[i].reg,
                                        vl53l0xInitSequence[i].value) != LASER_DISTANCE_OK)
        {
            log_error("LASER_DISTANCE: Failed to write VL53L0X register 0x%02X",
                      vl53l0xInitSequence[i].reg);
            return LASER_DISTANCE_I2C_ERROR;
        }
    }

    log_debug("LASER_DISTANCE: VL53L0X initialization completed");
    return LASER_DISTANCE_OK;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_ReadRange(LASER_DISTANCE_Handle_t *hlaser)
{
    uint8_t rangeData[VL53L0X_RANGE_DATA_SIZE];
    uint32_t startTime;
    bool measurementReady = false;

    /* A failed trigger would otherwise be followed by a full timeout poll. */
    if (LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_SYSRANGE_START,
                                    VL53L0X_SYSRANGE_SINGLE_SHOT) != LASER_DISTANCE_OK)
    {
        return LASER_DISTANCE_I2C_ERROR;
    }

    startTime = HAL_GetTick();

    while ((HAL_GetTick() - startTime) < hlaser->config.measurementTimeout)
    {
        if (LASER_DISTANCE_VL53L0X_IsReady(hlaser))
        {
            measurementReady = true;
            break;
        }
        HAL_Delay(VL53L0X_POLL_INTERVAL_MS);
    }

    /* Without this the result registers still hold the previous measurement,
       which would be returned as a fresh reading. */
    if (!measurementReady)
    {
        return LASER_DISTANCE_TIMEOUT;
    }

    if (LASER_DISTANCE_I2C_ReadMulti(hlaser, VL53L0X_REG_RESULT_RANGE_STATUS,
                                     rangeData, VL53L0X_RANGE_DATA_SIZE) != LASER_DISTANCE_OK)
    {
        return LASER_DISTANCE_I2C_ERROR;
    }

    hlaser->lastMeasurement.rangeStatus = rangeData[VL53L0X_RANGE_OFFSET_STATUS];
    hlaser->lastMeasurement.signalRate =
        (uint16_t)((rangeData[VL53L0X_RANGE_OFFSET_SIGNAL + 1U] << 8) |
                    rangeData[VL53L0X_RANGE_OFFSET_SIGNAL]);
    hlaser->lastMeasurement.ambientRate =
        (uint16_t)((rangeData[VL53L0X_RANGE_OFFSET_AMBIENT + 1U] << 8) |
                    rangeData[VL53L0X_RANGE_OFFSET_AMBIENT]);
    hlaser->lastMeasurement.distance =
        (uint16_t)((rangeData[VL53L0X_RANGE_OFFSET_DISTANCE + 1U] << 8) |
                    rangeData[VL53L0X_RANGE_OFFSET_DISTANCE]);

    (void)LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_SYSRANGE_START, VL53L0X_SYSRANGE_STOP);

    return LASER_DISTANCE_OK;
}

bool LASER_DISTANCE_VL53L0X_IsReady(const LASER_DISTANCE_Handle_t *hlaser)
{
    uint8_t status;

    if (LASER_DISTANCE_I2C_ReadReg(hlaser, VL53L0X_REG_RESULT_INTERRUPT_STATUS,
                                   &status) != LASER_DISTANCE_OK)
    {
        return false;
    }

    return (status & VL53L0X_INTERRUPT_STATUS_MASK) != 0U;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_Calibrate(LASER_DISTANCE_Handle_t *hlaser)
{
    LASER_DISTANCE_StatusTypeDef status;

    /* Simplified: a full calibration also needs a reference surface. */
    status = LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD,
                                         VL53L0X_VCSEL_PERIOD_PRE_RANGE);
    if (status != LASER_DISTANCE_OK)
    {
        return status;
    }

    return LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD,
                                       VL53L0X_VCSEL_PERIOD_FINAL_RANGE);
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_SetAddress(LASER_DISTANCE_Handle_t *hlaser,
                                                               uint8_t newAddress)
{
    LASER_DISTANCE_StatusTypeDef status;

    /* The register holds the 7-bit address, the handle keeps the shifted form. */
    status = LASER_DISTANCE_I2C_WriteReg(hlaser, VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS,
                                         (uint8_t)(newAddress >> 1));
    if (status == LASER_DISTANCE_OK)
    {
        hlaser->config.i2cAddress = newAddress;
    }

    return status;
}
