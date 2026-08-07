/**
  ******************************************************************************
  * @file    ili9488_core.h
  * @brief   Lifecycle, orientation and full-screen operations
  ******************************************************************************
  */

#ifndef ILI9488_CORE_H
#define ILI9488_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ili9488_types.h"

/**
 * @brief   Bring up the panel on the given control lines.
 * @param   hili Pointer to ILI9488 handle
 * @retval  ILI9488_INVALID_PARAM if the handle or any port is NULL.
 */
ILI9488_StatusTypeDef ILI9488_Init(ILI9488_Handle_t *hili,
                                   GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                   GPIO_TypeDef *dc_port, uint16_t dc_pin,
                                   GPIO_TypeDef *rst_port, uint16_t rst_pin);

/**
 * @brief   Set the display orientation and swap width/height to match.
 * @note    Resets the text cursor, which the new geometry may place off screen.
 */
ILI9488_StatusTypeDef ILI9488_SetOrientation(ILI9488_Handle_t *hili,
                                             ILI9488_Orientation_t orientation);

/**
 * @brief   Fill the whole panel with one colour.
 */
ILI9488_StatusTypeDef ILI9488_Clear(ILI9488_Handle_t *hili, uint16_t color);

#ifdef __cplusplus
}
#endif

#endif /* ILI9488_CORE_H */
