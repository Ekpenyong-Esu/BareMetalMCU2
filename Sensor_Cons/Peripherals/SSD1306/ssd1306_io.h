/**
 ******************************************************************************
 * @file    ssd1306_io.h
 * @brief   SSD1306 I2C transport - internal to the driver
 * @details Owns the control byte and the 7-bit to 8-bit address shift, and
 *          stages transfers in the handle's txBuffer. Not part of ssd1306.h.
 ******************************************************************************
 */

#ifndef SSD1306_IO_H
#define SSD1306_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306_types.h"

/**
 * @brief   Register the panel on @p bus and confirm it acknowledges.
 * @note    Reads the 7-bit address from hssd->config.
 */
SSD1306_StatusTypeDef SSD1306_IO_Init(SSD1306_Handle_t *hssd, I2C_Bus_t *bus);

SSD1306_StatusTypeDef SSD1306_IO_WriteCommand(SSD1306_Handle_t *hssd, uint8_t command);

/**
 * @brief   Send a run of command bytes in one transfer.
 */
SSD1306_StatusTypeDef SSD1306_IO_WriteCommands(SSD1306_Handle_t *hssd, const uint8_t *commands,
                                               uint16_t count);

SSD1306_StatusTypeDef SSD1306_IO_WriteData(SSD1306_Handle_t *hssd, const uint8_t *data,
                                           uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_IO_H */
