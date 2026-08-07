/**
 * @file    lcd_charset.c
 * @brief   Character LCD user defined characters (CGRAM)
 */

/* Includes ------------------------------------------------------------------*/
#include "lcd_charset.h"
#include "lcd_cursor.h"
#include "lcd_print.h"
#include "lcd_io.h"

/* Public functions ----------------------------------------------------------*/

LCD_StatusTypeDef LCD_CreateChar(LCD_HandleTypeDef* handle, uint8_t location,
                                 const uint8_t* charmap)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (charmap == NULL || location >= LCD_CGRAM_SLOTS)
    {
        return LCD_INVALID_PARAM;
    }

    LCD_IO_WriteByte(handle, (uint8_t)(LCD_CMD_SET_CGRAM_ADDR | (location << 3)), 0);

    for (uint8_t i = 0; i < LCD_CGRAM_PATTERN_BYTES; i++)
    {
        LCD_IO_WriteByte(handle, charmap[i], 1);
    }

    /* Writing CGRAM leaves the address counter there, so put it back in DDRAM */
    return LCD_SetCursor(handle, handle->cursorCol, handle->cursorRow);
}

LCD_StatusTypeDef LCD_PrintCustomChar(LCD_HandleTypeDef* handle, uint8_t location)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (location >= LCD_CGRAM_SLOTS)
    {
        return LCD_INVALID_PARAM;
    }

    /* CGRAM slots occupy character codes 0..7, so this is an ordinary write
       and must keep the tracked cursor position in step. */
    return LCD_PrintChar(handle, (char)location);
}
