/**
 ******************************************************************************
 * @file    nokia5110_io.h
 * @brief   SPI and control-line transport for the Nokia 5110 LCD
 * @note    Internal to the driver; not part of nokia5110.h. Every call reads
 *          the wiring and the bus slot from the handle.
 ******************************************************************************
 */

#ifndef NOKIA5110_IO_H
#define NOKIA5110_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nokia5110_types.h"

/**
 * @brief Configure the RST/CE/DC lines and register on the application's bus.
 * @note  Reads the wiring from @p hnok->Config, which must already be set.
 */
NOKIA5110_StatusTypeDef NOKIA5110_IO_Init(NOKIA5110_Handle_t *hnok);

/**
 * @brief Release the control lines.
 */
void NOKIA5110_IO_DeInit(NOKIA5110_Handle_t *hnok);

/**
 * @brief Pulse RST and leave the controller ready to accept commands.
 * @note  Does nothing when RST is not wired.
 */
void NOKIA5110_IO_Reset(NOKIA5110_Handle_t *hnok);

/**
 * @brief Send one command byte with DC low.
 */
NOKIA5110_StatusTypeDef NOKIA5110_IO_WriteCommand(NOKIA5110_Handle_t *hnok, uint8_t cmd);

/**
 * @brief Send a run of display bytes with DC high.
 */
NOKIA5110_StatusTypeDef NOKIA5110_IO_WriteData(NOKIA5110_Handle_t *hnok, const uint8_t *data,
                                               uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* NOKIA5110_IO_H */
