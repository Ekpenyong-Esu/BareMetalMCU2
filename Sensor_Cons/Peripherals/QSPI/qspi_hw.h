/**
 ******************************************************************************
 * @file    qspi_hw.h
 * @brief   Chip select and bus registration for the serial NOR flash
 * @note    The SPI pins belong to the bus the application opened; the only
 *          pin this driver owns is the chip select named in its config.
 ******************************************************************************
 */

#ifndef QSPI_HW_H
#define QSPI_HW_H

#ifdef __cplusplus
extern "C" {
#endif

#include "qspi_types.h"

/**
 * @brief Configure the chip-select pin as an output and idle it high.
 */
QSPI_StatusTypeDef QSPI_HW_InitCS(QSPI_HandleStructTypeDef *hqspi);

/**
 * @brief Release the chip-select pin.
 */
void QSPI_HW_DeInitCS(QSPI_HandleStructTypeDef *hqspi);

/**
 * @brief Register the flash on its bus with the settings the part needs.
 * @note  Re-registering after a prescaler change is enough; the bus is
 *        reprogrammed on the next transfer.
 */
QSPI_StatusTypeDef QSPI_HW_RegisterDevice(QSPI_HandleStructTypeDef *hqspi);

/**
 * @brief Assert (true) or release (false) the flash chip select.
 */
void QSPI_ChipSelect(const QSPI_HandleStructTypeDef *hqspi, bool select);

#ifdef __cplusplus
}
#endif

#endif /* QSPI_HW_H */
