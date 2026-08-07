/**
 * @file    lcd_charset.h
 * @brief   Character LCD user defined characters (CGRAM)
 */

#ifndef __LCD_CHARSET_H__
#define __LCD_CHARSET_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lcd_types.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Store a custom 5x8 character pattern in CGRAM
 * @param   handle   Pointer to LCD handle
 * @param   location CGRAM slot, 0..LCD_CGRAM_SLOTS - 1
 * @param   charmap  LCD_CGRAM_PATTERN_BYTES row bitmaps, bit 0 is the rightmost dot
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CreateChar(LCD_HandleTypeDef* handle, uint8_t location,
                                 const uint8_t* charmap);

/**
 * @brief   Print a previously stored custom character
 * @param   handle   Pointer to LCD handle
 * @param   location CGRAM slot, 0..LCD_CGRAM_SLOTS - 1
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintCustomChar(LCD_HandleTypeDef* handle, uint8_t location);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_CHARSET_H__ */
