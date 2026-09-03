/**
  ******************************************************************************
  * @file    i2c_core.h
  * @brief   Setup and sharing of the I2C bus
  * @details I2C is a two-wire bus for sensors. This board has one I2C bus
  *          (I2C3) shared by many devices. Each device signs up with its
  *          own address and speed. The bus switches to that device's
  *          settings when it talks to it.
  ******************************************************************************
  */

#ifndef I2C_CORE_H
#define I2C_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_types.h"

/**
 * @brief  Get default I2C settings: 100 kHz, 7-bit address, no clock stretch
 * @note   Copy this and change only what your device needs.
 * @retval Default settings ready to use
 */
I2C_ConfigTypeDef I2C_ConfigDefault(void);

/**
 * @brief  Add a device to the shared I2C bus
 * @param  device: Place to store device info, kept by the caller
 * @param  address: Address of the device on the bus
 * @param  config: Bus speed and settings for this device
 * @retval I2C_OK if added, error code if not
 */
I2C_StatusTypeDef I2C_DeviceInit(I2C_Device_t *device, uint16_t address,
                                 const I2C_ConfigTypeDef *config);

/**
 * @brief  Give the bus to a device so it can talk
 * @param  device: Device that wants to use the bus
 * @note   Only changes bus settings if a different device was last used.
 *         So two reads in a row on the same device are fast.
 * @retval I2C_OK if ok, error code if not
 */
I2C_StatusTypeDef I2C_Select(I2C_Device_t *device);

/**
 * @brief  Turn off the I2C bus
 * @note   Devices stay saved and can use the bus again later.
 * @retval I2C_OK if ok, error code if not
 */
I2C_StatusTypeDef I2C_BusDeInit(void);

/**
 * @brief  Check if a device is ready to use
 * @param  device: Device to check
 * @retval true if ready, false if not
 */
bool I2C_DeviceIsReady(const I2C_Device_t *device);

/**
 * @brief  Turn a HAL result into an I2C driver result
 * @param  halStatus: Result from HAL call
 * @retval Matching I2C status code
 */
I2C_StatusTypeDef I2C_ConvertHALStatus(HAL_StatusTypeDef halStatus);

/**
 * @brief  Get the low-level HAL handle for the I2C bus
 * @note   Pick a device with I2C_Select first, or you get the last device's settings.
 * @retval Pointer to the HAL handle
 */
I2C_HandleTypeDef *I2C_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_CORE_H */
