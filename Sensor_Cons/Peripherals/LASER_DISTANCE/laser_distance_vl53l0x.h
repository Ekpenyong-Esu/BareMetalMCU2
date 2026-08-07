/**
  ******************************************************************************
  * @file    laser_distance_vl53l0x.h
  * @brief   VL53L0X specific register map and operations
  * @details Internal to the driver: selected by the public API through the
  *          sensor type. Not part of the public aggregator.
  ******************************************************************************
  */

#ifndef __LASER_DISTANCE_VL53L0X_H__
#define __LASER_DISTANCE_VL53L0X_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "laser_distance_types.h"

/* Exported constants --------------------------------------------------------*/

/* Register addresses */
#define VL53L0X_REG_SYSRANGE_START                       0x00U
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS              0x13U
#define VL53L0X_REG_RESULT_RANGE_STATUS                  0x14U
#define VL53L0X_REG_READOUT_AVERAGING_SAMPLE_PERIOD      0x30U
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD        0x50U
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD      0x70U
#define VL53L0X_REG_I2C_SLAVE_DEVICE_ADDRESS             0x8AU
#define VL53L0X_REG_IDENTIFICATION_MODEL_ID              0xC0U
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID           0xC2U

/* Register values */
#define VL53L0X_EXPECTED_DEVICE_ID                       0xEEU
#define VL53L0X_READOUT_AVERAGING_PERIOD                 0x30U
#define VL53L0X_VCSEL_PERIOD_PRE_RANGE                   0x18U
#define VL53L0X_VCSEL_PERIOD_FINAL_RANGE                 0x08U

/* SYSRANGE_START commands */
#define VL53L0X_SYSRANGE_STOP                            0x00U
#define VL53L0X_SYSRANGE_SINGLE_SHOT                     0x01U
#define VL53L0X_SYSRANGE_BACK_TO_BACK                    0x02U

/** @brief Bits [2:0] hold the range-complete code; non-zero means data ready */
#define VL53L0X_INTERRUPT_STATUS_MASK                    0x07U

/** @brief Bytes read in one go starting at RESULT_RANGE_STATUS */
#define VL53L0X_RANGE_DATA_SIZE                          12U

/* 8-bit (shifted) I2C addresses */
#define LASER_DISTANCE_VL53L0X_DEFAULT_ADDR              0x52U
#define LASER_DISTANCE_VL53L0X_ALT_ADDR                  0x54U

/* Exported functions prototypes ---------------------------------------------*/

/**
 * @brief   Identify the device and apply the base configuration
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_Init(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Run one single-shot measurement and store the result
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_ReadRange(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Report whether a range result is waiting to be read
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  bool True when the range-complete interrupt is set
 */
bool LASER_DISTANCE_VL53L0X_IsReady(const LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Re-apply the VCSEL periods
 * @param   hlaser Pointer to laser distance sensor handle
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_Calibrate(LASER_DISTANCE_Handle_t *hlaser);

/**
 * @brief   Move the device to another I2C address
 * @param   hlaser Pointer to laser distance sensor handle
 * @param   newAddress New address in 8-bit (shifted) form
 * @retval  LASER_DISTANCE_StatusTypeDef Operation status
 */
LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_VL53L0X_SetAddress(LASER_DISTANCE_Handle_t *hlaser,
                                                               uint8_t newAddress);

#ifdef __cplusplus
}
#endif

#endif /* __LASER_DISTANCE_VL53L0X_H__ */
