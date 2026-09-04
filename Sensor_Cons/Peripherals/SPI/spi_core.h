/**
 ******************************************************************************
 * @file    spi_core.h
 * @brief   Bus lifecycle, device registration and ownership for SPI
 * @details The application opens a bus with SPI_BusInit, naming the
 *          peripheral and its pins. Device drivers register on that bus
 *          with SPI_DeviceInit and the settings their chip needs. The bus
 *          is reprogrammed whenever ownership moves between devices, so
 *          nothing here assumes which chips are wired to which bus.
 ******************************************************************************
 */

#ifndef SPI_CORE_H
#define SPI_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spi_types.h"

/**
 * @brief   Bring up the pins and clock of one SPI peripheral
 * @param   bus    Caller-owned bus record
 * @param   config Peripheral and wiring
 * @retval  SPI_StatusTypeDef Status of the operation
 * @note    The peripheral itself is programmed on first use, because its
 *          settings belong to whichever device is selected.
 */
SPI_StatusTypeDef SPI_BusInit(SPI_Bus_t *bus, const SPI_BusConfig_t *config);

/**
 * @brief   Stop the peripheral and release its pins
 * @param   bus Bus to close
 * @retval  SPI_StatusTypeDef Status of the operation
 * @note    Registered devices keep pointing at the bus; reopen it and they
 *          work again.
 */
SPI_StatusTypeDef SPI_BusDeInit(SPI_Bus_t *bus);

/**
 * @brief   Whether a bus has been opened
 */
bool SPI_BusIsReady(const SPI_Bus_t *bus);

/**
 * @brief   Settings most chips accept
 * @retval  SPI_ConfigTypeDef Master, 8-bit, mode 0, software NSS, PCLK/8
 * @note    Intended to be copied and adjusted, so a device only has to state
 *          the fields where it differs.
 */
SPI_ConfigTypeDef SPI_ConfigDefault(void);

/**
 * @brief   Register a device on a bus with the settings it needs
 * @param   device Caller-owned device record
 * @param   bus    Bus the chip is wired to, already opened
 * @param   config Settings to apply whenever this device is selected
 * @retval  SPI_StatusTypeDef Status of the operation
 * @note    Does not touch the peripheral; the bus is programmed on first use.
 */
SPI_StatusTypeDef SPI_DeviceInit(SPI_Device_t *device, SPI_Bus_t *bus,
                                 const SPI_ConfigTypeDef *config);

/**
 * @brief   Give a device ownership of its bus
 * @param   device Device to select
 * @retval  SPI_StatusTypeDef Status of the operation
 * @note    Reprograms the peripheral only when ownership actually changes, so
 *          repeated transfers to the same device cost nothing extra. The
 *          transfer functions call this, so drivers rarely need to.
 */
SPI_StatusTypeDef SPI_Select(SPI_Device_t *device);

/**
 * @brief   Whether a device has been registered on an open bus
 */
bool SPI_DeviceIsReady(const SPI_Device_t *device);

/**
 * @brief   Map a HAL result onto the driver's status enum
 */
SPI_StatusTypeDef SPI_ConvertHALStatus(HAL_StatusTypeDef halStatus);

/**
 * @brief   Underlying HAL handle of the bus a device sits on
 * @retval  SPI_HandleTypeDef* Handle reflecting whichever device is selected,
 *          or NULL if the device is not registered
 */
SPI_HandleTypeDef *SPI_GetHandle(const SPI_Device_t *device);

#ifdef __cplusplus
}
#endif

#endif /* SPI_CORE_H */
