/**
 * @file    lcd_cursor.h
 * @brief   Character LCD cursor positioning and appearance
 */

#ifndef LCD_CURSOR_H
#define LCD_CURSOR_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lcd_types.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Move the cursor to a column and row
 * @param   handle Pointer to LCD handle
 * @param   col    Column index, 0 based
 * @param   row    Row index, 0 based
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_SetCursor(LCD_HandleTypeDef *handle, uint8_t col, uint8_t row);

/**
 * @brief   Show the cursor underline
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CursorOn(LCD_HandleTypeDef *handle);

/**
 * @brief   Hide the cursor underline
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CursorOff(LCD_HandleTypeDef *handle);

/**
 * @brief   Enable cursor blinking
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_BlinkOn(LCD_HandleTypeDef *handle);

/**
 * @brief   Disable cursor blinking
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_BlinkOff(LCD_HandleTypeDef *handle);

/**
 * @brief   Move the cursor one position to the left
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CursorLeft(LCD_HandleTypeDef *handle);

/**
 * @brief   Move the cursor one position to the right
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_CursorRight(LCD_HandleTypeDef *handle);

#ifdef __cplusplus
}
#endif

#endif /* LCD_CURSOR_H */
