/**
 * @file    ili9341_io.h
 * @brief   ILI9341 SPI command and data transport
 */

#ifndef ILI9341_IO_H
#define ILI9341_IO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "ili9341_types.h"

/* Exported functions --------------------------------------------------------*/

/** @brief Send one controller command byte. */
void ili9341_WriteReg(uint8_t command);

/** @brief Send one controller parameter byte. */
void ili9341_WriteData(uint8_t data);

/** @brief Read up to four bytes returned by a controller command. */
uint32_t ili9341_ReadData(uint16_t command, uint8_t readSize);

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_IO_H */
