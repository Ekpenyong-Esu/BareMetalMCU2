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

/**
 * @brief Configure the control lines, claim a slot on the bus and pulse reset
 * @param hili Handle whose config names the bus and pins
 * @return SPI_StatusTypeDef SPI_INVALID_PARAM if the bus or a required port is
 *         NULL, otherwise the result of registering on the bus
 * @note   Called by ILI9341_Init; applications do not need it.
 */
SPI_StatusTypeDef ILI9341_IO_Init(ILI9341_Handle_t *hili);

/** @brief Return the control lines to their idle state. */
void ILI9341_IO_DeInit(ILI9341_Handle_t *hili);

/**
 * @brief Send one controller command byte
 * @param hili Handle
 * @param command Controller command opcode
 * @return SPI_StatusTypeDef Result of the underlying bus transfer
 */
SPI_StatusTypeDef ILI9341_WriteReg(ILI9341_Handle_t *hili, uint8_t command);

/**
 * @brief Send one controller parameter byte
 * @param hili Handle
 * @param data Parameter belonging to the command last written
 * @return SPI_StatusTypeDef Result of the underlying bus transfer
 */
SPI_StatusTypeDef ILI9341_WriteData(ILI9341_Handle_t *hili, uint8_t data);

/**
 * @brief Read up to four bytes returned by a controller command
 * @param hili Handle
 * @param command Controller command opcode
 * @param readSize Bytes to read back, clamped to four
 * @return uint32_t Response bytes, most significant first; zero on a bus error,
 *         which is indistinguishable from a genuine zero reply.
 */
uint32_t ILI9341_ReadData(ILI9341_Handle_t *hili, uint16_t command, uint8_t readSize);

#ifdef __cplusplus
}
#endif

#endif /* ILI9341_IO_H */
