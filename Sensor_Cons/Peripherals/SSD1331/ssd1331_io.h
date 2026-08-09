/**
  ******************************************************************************
  * @file    ssd1331_io.h
  * @brief   SSD1331 SPI transport - internal to the driver
  * @details Owns the chip-select and data/command lines and the bus settings.
  *          Not part of ssd1331.h.
  ******************************************************************************
  */

#ifndef SSD1331_IO_H
#define SSD1331_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1331_types.h"

/**
 * @brief   Bring up the control pins and the SPI bus, then reset the panel.
 */
SSD1331_StatusTypeDef SSD1331_IO_Init(const SSD1331_Config_t *config);

SSD1331_StatusTypeDef SSD1331_IO_WriteCommand(const SSD1331_Config_t *config, uint8_t command);

/**
 * @brief   Send a run of command bytes in one chip-select assertion.
 */
SSD1331_StatusTypeDef SSD1331_IO_WriteCommands(const SSD1331_Config_t *config,
                                               const uint8_t *commands, uint16_t count);

/**
 * @brief   Stream pixel data to the address window set by the last command.
 * @note    Takes a mutable buffer because the SPI driver transmits in place.
 */
SSD1331_StatusTypeDef SSD1331_IO_WriteData(const SSD1331_Config_t *config,
                                           uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* SSD1331_IO_H */
