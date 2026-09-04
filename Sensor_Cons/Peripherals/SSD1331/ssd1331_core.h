/**
 ******************************************************************************
 * @file    ssd1331_core.h
 * @brief   SSD1331 lifecycle and panel control
 ******************************************************************************
 */

#ifndef SSD1331_CORE_H
#define SSD1331_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ssd1331_types.h"

/**
 * @brief   Register on the bus, bring up the panel and leave a blank screen displayed.
 * @param   config Bus, prescaler and control pins; copied into the handle.
 *                 The bus must already be open (SPI_BusInit).
 */
SSD1331_StatusTypeDef SSD1331_Init(SSD1331_Handle_t *hssd, const SSD1331_Config_t *config);

SSD1331_StatusTypeDef SSD1331_DeInit(SSD1331_Handle_t *hssd);

SSD1331_StatusTypeDef SSD1331_DisplayOn(SSD1331_Handle_t *hssd, bool enable);

/**
 * @brief   Set the per-channel contrast.
 * @note    The panel has no single contrast register; each colour drives its
 *          own current, so all three have to be given.
 */
SSD1331_StatusTypeDef SSD1331_SetContrast(SSD1331_Handle_t *hssd, uint8_t red, uint8_t green,
                                          uint8_t blue);

/**
 * @brief   Scale every channel at once, 0 (dimmest) to 15 (brightest).
 */
SSD1331_StatusTypeDef SSD1331_SetBrightness(SSD1331_Handle_t *hssd, uint8_t current);

SSD1331_StatusTypeDef SSD1331_InvertDisplay(SSD1331_Handle_t *hssd, bool invert);

#ifdef __cplusplus
}
#endif

#endif /* SSD1331_CORE_H */
