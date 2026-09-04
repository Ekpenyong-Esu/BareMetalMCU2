/**
 * @file    lcd_display.h
 * @brief   Character LCD display level control: clearing, power and scrolling
 */

#ifndef LCD_DISPLAY_H
#define LCD_DISPLAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lcd_types.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Clear entire display and return the cursor home
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_Clear(LCD_HandleTypeDef *handle);

/**
 * @brief   Return cursor to the home position without clearing
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_Home(LCD_HandleTypeDef *handle);

/**
 * @brief   Turn the display on
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_DisplayOn(LCD_HandleTypeDef *handle);

/**
 * @brief   Turn the display off, retaining its contents
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_DisplayOff(LCD_HandleTypeDef *handle);

/**
 * @brief   Turn the backlight on
 * @note    Does nothing when the configuration has no backlight pin.
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_BacklightOn(LCD_HandleTypeDef *handle);

/**
 * @brief   Turn the backlight off
 * @note    Does nothing when the configuration has no backlight pin.
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_BacklightOff(LCD_HandleTypeDef *handle);

/**
 * @brief   Overwrite one row with spaces and leave the cursor at its start
 * @param   handle Pointer to LCD handle
 * @param   row    Row index
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_ClearLine(LCD_HandleTypeDef *handle, uint8_t row);

/**
 * @brief   Shift the whole display one position to the left
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_ScrollLeft(LCD_HandleTypeDef *handle);

/**
 * @brief   Shift the whole display one position to the right
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_ScrollRight(LCD_HandleTypeDef *handle);

/**
 * @brief   Shift the display on every character write
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_AutoScrollOn(LCD_HandleTypeDef *handle);

/**
 * @brief   Keep the display still on character writes
 * @param   handle Pointer to LCD handle
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_AutoScrollOff(LCD_HandleTypeDef *handle);

#ifdef __cplusplus
}
#endif

#endif /* LCD_DISPLAY_H */
