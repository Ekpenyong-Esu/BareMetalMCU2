/**
 ******************************************************************************
 * @file    i2c_core.h
 * @brief   Bus lifecycle, device registration and ownership for I2C
 * @details The application opens a bus with I2C_BusInit, naming the
 *          peripheral and its pins. Device drivers register on that bus
 *          with I2C_DeviceInit, giving the chip's address and speed. The
 *          bus switches to a device's settings when it talks to it, so
 *          nothing here assumes which chips are wired to which bus.
 ******************************************************************************
 */

#ifndef I2C_CORE_H
#define I2C_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "i2c_types.h"

/**
 * @brief  Bring up the pins and clock of one I2C peripheral
 * @param  bus: Caller-owned bus record
 * @param  config: Peripheral and wiring
 * @retval I2C_OK if opened, error code if not
 * @note   The peripheral itself is programmed on first use, because its
 *         speed belongs to whichever device is selected.
 */
I2C_StatusTypeDef I2C_BusInit(I2C_Bus_t *bus, const I2C_BusConfig_t *config);

/**
 * @brief  Stop the peripheral and release its pins
 * @param  bus: Bus to close
 * @note   Registered devices keep pointing at the bus; reopen it and they
 *         work again.
 * @retval I2C_OK if ok, error code if not
 */
I2C_StatusTypeDef I2C_BusDeInit(I2C_Bus_t *bus);

/**
 * @brief  Whether a bus has been opened
 */
bool I2C_BusIsReady(const I2C_Bus_t *bus);

/**
 * @brief  Put the bus pins (back) into alternate-function mode
 * @note   Internal to the driver: the transfer layer calls it after it has
 *         bit-banged the wires free.
 */
void I2C_BusInitPins(const I2C_Bus_t *bus);

/**
 * @brief  Get default I2C settings: 100 kHz, 7-bit address, no clock stretch
 * @note   Copy this and change only what your device needs.
 * @retval Default settings ready to use
 */
I2C_ConfigTypeDef I2C_ConfigDefault(void);

/**
 * @brief  Register a device on a bus
 * @param  device: Place to store device info, kept by the caller
 * @param  bus: Bus the chip is wired to, already opened
 * @param  address: Address of the chip on the bus (7-bit address shifted left)
 * @param  config: Bus speed and settings for this chip
 * @retval I2C_OK if added, error code if not
 */
I2C_StatusTypeDef I2C_DeviceInit(I2C_Device_t *device, I2C_Bus_t *bus, uint16_t address,
                                 const I2C_ConfigTypeDef *config);

/**
 * @brief  Give the bus to a device so it can talk
 * @param  device: Device that wants to use its bus
 * @note   Only changes bus settings if a different device was last used.
 *         So two reads in a row on the same device are fast.
 * @retval I2C_OK if ok, error code if not
 */
I2C_StatusTypeDef I2C_Select(I2C_Device_t *device);

/**
 * @brief  Check if a device is registered on an open bus
 */
bool I2C_DeviceIsReady(const I2C_Device_t *device);

/**
 * @brief  Turn a HAL result into an I2C driver result
 */
I2C_StatusTypeDef I2C_ConvertHALStatus(HAL_StatusTypeDef halStatus);

/**
 * @brief  Get the low-level HAL handle of the bus a device sits on
 * @note   Pick the device with I2C_Select first, or you get the last device's settings.
 * @retval Pointer to the HAL handle, or NULL if the device is not registered
 */
I2C_HandleTypeDef *I2C_GetHandle(const I2C_Device_t *device);

#ifdef __cplusplus
}
#endif

#endif /* I2C_CORE_H */
