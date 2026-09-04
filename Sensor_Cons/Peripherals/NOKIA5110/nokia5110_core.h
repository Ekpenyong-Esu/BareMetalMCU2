/**
 ******************************************************************************
 * @file    nokia5110_core.h
 * @brief   Lifecycle, configuration and framebuffer transfer
 ******************************************************************************
 */

#ifndef NOKIA5110_CORE_H
#define NOKIA5110_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nokia5110_types.h"

/**
 * @brief   Bring up the display on the given bus and pins and apply @p config.
 * @param   hnok   Pointer to Nokia 5110 handle
 * @param   config Bus, control lines and display settings; copied into the
 *                 handle. Start from NOKIA5110_GetDefaultConfig() and fill in
 *                 the wiring. The bus must already be open (SPI_BusInit).
 * @retval  NOKIA5110_StatusTypeDef Operation status
 */
NOKIA5110_StatusTypeDef NOKIA5110_Init(NOKIA5110_Handle_t *hnok, const NOKIA5110_Config_t *config);

/**
 * @brief   Power the display down and release the control lines.
 */
NOKIA5110_StatusTypeDef NOKIA5110_DeInit(NOKIA5110_Handle_t *hnok);

/**
 * @brief   Apply contrast, temperature coefficient, bias and display mode.
 * @retval  NOKIA5110_INVALID_PARAM if any field is out of range.
 * @note    The wiring fields of @p config are ignored; they are fixed at Init.
 */
NOKIA5110_StatusTypeDef NOKIA5110_Config(NOKIA5110_Handle_t *hnok,
                                         const NOKIA5110_Config_t *config);

/**
 * @brief   Clear the framebuffer and push it to the display.
 */
NOKIA5110_StatusTypeDef NOKIA5110_Clear(NOKIA5110_Handle_t *hnok);

/**
 * @brief   Push the framebuffer to the display.
 */
NOKIA5110_StatusTypeDef NOKIA5110_Update(NOKIA5110_Handle_t *hnok);

/**
 * @brief   Set the display contrast (Vop), 0-127.
 */
NOKIA5110_StatusTypeDef NOKIA5110_SetContrast(NOKIA5110_Handle_t *hnok, uint8_t contrast);

/**
 * @brief   Switch between blank, normal, all-on and inverse.
 */
NOKIA5110_StatusTypeDef NOKIA5110_SetMode(NOKIA5110_Handle_t *hnok, NOKIA5110_DisplayMode_t mode);

uint8_t NOKIA5110_GetWidth(void);
uint8_t NOKIA5110_GetHeight(void);

/**
 * @brief   Display settings most panels accept, with the wiring left NULL.
 */
NOKIA5110_Config_t NOKIA5110_GetDefaultConfig(void);

#ifdef __cplusplus
}
#endif

#endif /* NOKIA5110_CORE_H */
