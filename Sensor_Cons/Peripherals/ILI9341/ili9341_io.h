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
#include "spi_types.h"

/* Exported functions --------------------------------------------------------*/

/** @brief Claim the panel's slot on the shared SPI bus. */
SPI_StatusTypeDef ILI9341_IO_BusInit(void);

/**
 * @brief Send one controller command byte
 * @param command Controller command opcode
 * @return SPI_StatusTypeDef Result of the underlying bus transfer
 */
SPI_StatusTypeDef ili9341_WriteReg(uint8_t command);

/**
 * @brief Send one controller parameter byte
 * @param data Parameter belonging to the command last written
 * @return SPI_StatusTypeDef Result of the underlying bus transfer
 */
SPI_StatusTypeDef ili9341_WriteData(uint8_t data);

/**
 * @brief Read up to four bytes returned by a controller command
 * @param command Controller command opcode
 * @param readSize Bytes to read back, clamped to four
 * @return uint32_t Response bytes, most significant first; zero on a bus error,
 *         which is indistinguishable from a genuine zero reply.
 */
uint32_t ili9341_ReadData(uint16_t command, uint8_t readSize);

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_IO_H */
