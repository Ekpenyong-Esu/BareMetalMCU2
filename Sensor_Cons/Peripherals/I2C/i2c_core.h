/**
  ******************************************************************************
  * @file    i2c_core.h
  * @brief   Bus lifecycle and per-device registration for the I2C driver
  * @details The board exposes one usable I2C bus (I2C3) but several devices sit
  *          on it. Each driver registers its own I2C_Device_t, and the bus is
  *          reprogrammed to that device's settings when a transfer starts.
  ******************************************************************************
  */

#ifndef I2C_CORE_H
#define I2C_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_types.h"

/**
 * @brief Board defaults: 100 kHz standard mode, 7-bit addressing, no stretching
 * @note  Copy this and override only the fields the device disagrees with.
 */
I2C_ConfigTypeDef I2C_ConfigDefault(void);

/**
 * @brief Register a device on the bus
 * @param device  Storage owned by the calling driver, valid for its lifetime
 * @param address Slave address, already shifted the way the HAL expects it
 * @param config  Bus settings this device needs
 */
I2C_StatusTypeDef I2C_DeviceInit(I2C_Device_t *device, uint16_t address,
                                 const I2C_ConfigTypeDef *config);

/**
 * @brief Hand the bus to a device, reprogramming it only if the owner changed
 * @note  Called by every transfer, so back-to-back reads on one device cost
 *        nothing extra.
 */
I2C_StatusTypeDef I2C_Select(I2C_Device_t *device);

/**
 * @brief Release the bus; devices stay registered and reclaim it on next use.
 */
I2C_StatusTypeDef I2C_BusDeInit(void);

/**
 * @brief True if the device has been registered and its config was accepted.
 */
bool I2C_DeviceIsReady(const I2C_Device_t *device);

/**
 * @brief Map a HAL result onto the driver's status enum.
 */
I2C_StatusTypeDef I2C_ConvertHALStatus(HAL_StatusTypeDef halStatus);

/**
 * @brief Access the HAL handle, for consumers that must pass it to their own
 *        init. Select a device first, or the settings are whoever ran last.
 */
I2C_HandleTypeDef *I2C_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* I2C_CORE_H */
