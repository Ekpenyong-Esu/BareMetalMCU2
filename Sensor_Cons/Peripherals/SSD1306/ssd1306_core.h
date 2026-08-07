/**
  ******************************************************************************
  * @file    ssd1306_core.h
  * @brief   SSD1306 lifecycle and panel control
  ******************************************************************************
  */

#ifndef SSD1306_CORE_H
#define SSD1306_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1306_types.h"

/**
 * @brief   Bring up the bus and the panel and leave a blank screen displayed.
 * @param   address 7-bit address, SSD1306_I2C_ADDR or SSD1306_I2C_ADDR_ALT.
 */
SSD1306_StatusTypeDef SSD1306_Init(SSD1306_Handle_t *hssd, uint8_t address);

SSD1306_StatusTypeDef SSD1306_DeInit(SSD1306_Handle_t *hssd);

SSD1306_StatusTypeDef SSD1306_DisplayOn(SSD1306_Handle_t *hssd, bool on);

SSD1306_StatusTypeDef SSD1306_SetContrast(SSD1306_Handle_t *hssd, uint8_t contrast);

SSD1306_StatusTypeDef SSD1306_InvertDisplay(SSD1306_Handle_t *hssd, bool invert);

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_CORE_H */
