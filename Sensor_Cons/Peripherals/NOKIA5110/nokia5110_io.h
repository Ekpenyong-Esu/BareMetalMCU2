/**
  ******************************************************************************
  * @file    nokia5110_io.h
  * @brief   SPI and control-line transport for the Nokia 5110 LCD
  * @note    Internal to the driver; not part of nokia5110.h.
  ******************************************************************************
  */

#ifndef NOKIA5110_IO_H
#define NOKIA5110_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nokia5110_types.h"

/**
 * @brief Configure the RST/CE/DC lines and bring up the SPI bus.
 */
NOKIA5110_StatusTypeDef NOKIA5110_IO_Init(void);

/**
 * @brief Release the control lines.
 */
void NOKIA5110_IO_DeInit(void);

/**
 * @brief Pulse RST and leave the controller ready to accept commands.
 */
void NOKIA5110_IO_Reset(void);

/**
 * @brief Send one command byte with DC low.
 */
NOKIA5110_StatusTypeDef NOKIA5110_IO_WriteCommand(uint8_t cmd);

/**
 * @brief Send a run of display bytes with DC high.
 */
NOKIA5110_StatusTypeDef NOKIA5110_IO_WriteData(const uint8_t *data, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* NOKIA5110_IO_H */
