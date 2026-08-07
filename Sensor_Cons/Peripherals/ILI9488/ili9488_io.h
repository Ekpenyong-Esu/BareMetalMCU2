/**
  ******************************************************************************
  * @file    ili9488_io.h
  * @brief   SPI and control-line transport for the ILI9488
  * @note    Internal to the driver; not part of ili9488.h.
  ******************************************************************************
  */

#ifndef ILI9488_IO_H
#define ILI9488_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ili9488_types.h"

/**
 * @brief Configure the CS/DC/RST lines, bring up SPI and pulse the panel reset.
 */
ILI9488_StatusTypeDef ILI9488_IO_Init(const ILI9488_Config_t *config);

/**
 * @brief Send one command byte with DC low.
 */
ILI9488_StatusTypeDef ILI9488_IO_WriteCommand(const ILI9488_Config_t *config, uint8_t command);

/**
 * @brief Send a run of parameter bytes with DC high.
 */
ILI9488_StatusTypeDef ILI9488_IO_WriteData(const ILI9488_Config_t *config,
                                           const uint8_t *data, uint16_t size);

/**
 * @brief Repeat one RGB565 colour into display memory.
 * @note  Expands to the 18-bit format the panel requires over SPI.
 */
ILI9488_StatusTypeDef ILI9488_IO_WritePixels(const ILI9488_Config_t *config,
                                             uint16_t color, uint32_t count);

/**
 * @brief Stream a run of RGB565 colours into display memory.
 * @note  Expands to the 18-bit format the panel requires over SPI.
 */
ILI9488_StatusTypeDef ILI9488_IO_WritePixelBuffer(const ILI9488_Config_t *config,
                                                  const uint16_t *colors, uint32_t count);

/**
 * @brief Select the rectangle that subsequent memory writes fill.
 */
ILI9488_StatusTypeDef ILI9488_IO_SetAddressWindow(const ILI9488_Config_t *config,
                                                  uint16_t x0, uint16_t y0,
                                                  uint16_t x1, uint16_t y1);

#ifdef __cplusplus
}
#endif

#endif /* ILI9488_IO_H */
