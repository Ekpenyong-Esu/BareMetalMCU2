/**
 ******************************************************************************
 * @file    ssd1331_io.h
 * @brief   SSD1331 SPI transport - internal to the driver
 * @details Owns the chip-select and data/command lines and the panel's slot
 *          on the application's SPI bus. Not part of ssd1331.h.
 ******************************************************************************
 */

#ifndef SSD1331_IO_H
#define SSD1331_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1331_types.h"

/**
 * @brief   Bring up the control pins, register on the bus, then reset the panel.
 * @note    Reads the wiring from @p hssd->config, which must already be set.
 */
SSD1331_StatusTypeDef SSD1331_IO_Init(SSD1331_Handle_t *hssd);

SSD1331_StatusTypeDef SSD1331_IO_WriteCommand(SSD1331_Handle_t *hssd, uint8_t command);

/**
 * @brief   Send a run of command bytes in one chip-select assertion.
 */
SSD1331_StatusTypeDef SSD1331_IO_WriteCommands(SSD1331_Handle_t *hssd, const uint8_t *commands,
                                               uint16_t count);

/**
 * @brief   Stream pixel data to the address window set by the last command.
 * @note    Takes a mutable buffer because the SPI driver transmits in place.
 */
SSD1331_StatusTypeDef SSD1331_IO_WriteData(SSD1331_Handle_t *hssd, uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* SSD1331_IO_H */
