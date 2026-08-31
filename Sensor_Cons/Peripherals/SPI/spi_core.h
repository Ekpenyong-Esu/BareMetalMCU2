/**
  ******************************************************************************
  * @file    spi_core.h
  * @brief   Device registration and bus ownership for the SPI driver
  * @details The STM32F429I-DISC1 wires one usable SPI bus (SPI5) to several
  *          devices: the ILI9341 display and the L3GD20 gyroscope on board,
  *          plus whatever is added off board. Each device declares the bus
  *          settings it needs, and the bus is reprogrammed when ownership
  *          moves between them.
  ******************************************************************************
  */

#ifndef SPI_CORE_H
#define SPI_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "spi_types.h"

/**
 * @brief   Bus settings shared by most devices on this board
 * @retval  SPI_ConfigTypeDef Master, 8-bit, mode 0, software NSS, PCLK2/8
 * @note    Intended to be copied and adjusted, so a device only has to state
 *          the fields where it differs.
 */
SPI_ConfigTypeDef SPI_ConfigDefault(void);

/**
 * @brief   Register a device and the bus settings it needs
 * @param   device Caller-owned device record
 * @param   config Settings to apply whenever this device is selected
 * @retval  SPI_StatusTypeDef Status of the operation
 * @note    Does not touch the peripheral; the bus is programmed on first use.
 */
SPI_StatusTypeDef SPI_DeviceInit(SPI_Device_t *device, const SPI_ConfigTypeDef *config);

/**
 * @brief   Give a device ownership of the bus
 * @param   device Device to select
 * @retval  SPI_StatusTypeDef Status of the operation
 * @note    Reprograms the peripheral only when ownership actually changes, so
 *          repeated transfers to the same device cost nothing extra. The
 *          transfer functions call this, so drivers rarely need to.
 */
SPI_StatusTypeDef SPI_Select(SPI_Device_t *device);

/**
 * @brief   Release the bus and stop the peripheral
 * @retval  SPI_StatusTypeDef Status of the operation
 */
SPI_StatusTypeDef SPI_BusDeInit(void);

/**
 * @brief   Whether a device has been registered
 * @param   device Device to test
 * @retval  bool true when usable
 */
bool SPI_DeviceIsReady(const SPI_Device_t *device);

/**
 * @brief   Map a HAL result onto the driver's status enum
 */
SPI_StatusTypeDef SPI_ConvertHALStatus(HAL_StatusTypeDef halStatus);

/**
 * @brief   Underlying HAL handle for the shared bus
 * @retval  SPI_HandleTypeDef* Handle reflecting whichever device is selected
 */
SPI_HandleTypeDef *SPI_GetHandle(void);

#ifdef __cplusplus
}
#endif

#endif /* SPI_CORE_H */
