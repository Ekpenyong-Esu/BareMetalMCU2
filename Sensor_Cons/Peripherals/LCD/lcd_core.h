/**
 * @file    lcd_core.h
 * @brief   Character LCD lifecycle, geometry and raw bus access
 */

#ifndef __LCD_CORE_H__
#define __LCD_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lcd_types.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Initialize LCD with configuration
 * @param   handle Pointer to LCD handle
 * @param   config Pointer to configuration structure
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_Init(LCD_HandleTypeDef* handle, const LCD_ConfigTypeDef* config);

/**
 * @brief   Initialize LCD with a default 16x2, 4-bit configuration
 * @param   handle Pointer to LCD handle
 * @param   pins   Pointer to pin assignments
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_InitDefault(LCD_HandleTypeDef* handle, const LCD_PinsTypeDef* pins);

/**
 * @brief   Deinitialize LCD
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_DeInit(LCD_HandleTypeDef* handle);

/**
 * @brief   Send a raw command byte to the panel
 * @param   handle Pointer to LCD handle
 * @param   cmd    Command byte
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_SendCommand(LCD_HandleTypeDef* handle, uint8_t cmd);

/**
 * @brief   Send a raw data byte to the panel
 * @param   handle Pointer to LCD handle
 * @param   data   Data byte
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_SendData(LCD_HandleTypeDef* handle, uint8_t data);

/**
 * @brief   Re-send the display control command from the handle's state flags
 * @note    Internal to the driver; used by the display and cursor modules.
 * @param   handle Pointer to LCD handle
 */
void LCD_UpdateDisplayControl(LCD_HandleTypeDef* handle);

/**
 * @brief   DDRAM start address of each row for the configured display size
 * @note    Internal to the driver; the returned array has LCD_MAX_ROWS entries.
 * @param   size Display size type
 * @retval  Pointer to the row offset table
 */
const uint8_t* LCD_GetRowOffsets(LCD_SizeTypeDef size);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_CORE_H__ */
