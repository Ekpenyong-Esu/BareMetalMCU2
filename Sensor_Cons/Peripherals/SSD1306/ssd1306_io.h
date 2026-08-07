/**
  ******************************************************************************
  * @file    ssd1306_io.h
  * @brief   SSD1306 I2C transport - internal to the driver
  * @details Owns the control byte, the 7-bit to 8-bit address shift and the
  *          staging buffer. Not part of ssd1306.h.
  ******************************************************************************
  */

#ifndef SSD1306_IO_H
#define SSD1306_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306_types.h"

/**
 * @brief   Bring up the I2C bus and confirm the panel acknowledges.
 */
SSD1306_StatusTypeDef SSD1306_IO_Init(const SSD1306_Config_t *config);

SSD1306_StatusTypeDef SSD1306_IO_WriteCommand(const SSD1306_Config_t *config, uint8_t command);

/**
 * @brief   Send a run of command bytes in one transfer.
 */
SSD1306_StatusTypeDef SSD1306_IO_WriteCommands(const SSD1306_Config_t *config,
                                               const uint8_t *commands, uint16_t count);

SSD1306_StatusTypeDef SSD1306_IO_WriteData(const SSD1306_Config_t *config,
                                           const uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_IO_H */
