/**
 * @file    lcd_cursor.c
 * @brief   Character LCD cursor positioning and appearance
 */

/* Includes ------------------------------------------------------------------*/
#include "lcd_cursor.h"
#include "lcd_core.h"
#include "lcd_io.h"

/* Public functions ----------------------------------------------------------*/

LCD_StatusTypeDef LCD_SetCursor(LCD_HandleTypeDef* handle, uint8_t col, uint8_t row)
{
    const uint8_t* rowOffsets;

    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (col >= handle->cols || row >= handle->rows)
    {
        return LCD_INVALID_PARAM;
    }

    rowOffsets = LCD_GetRowOffsets(handle->config.size);

    LCD_IO_WriteByte(handle, (uint8_t)(LCD_CMD_SET_DDRAM_ADDR | (col + rowOffsets[row])), 0);

    handle->cursorCol = col;
    handle->cursorRow = row;

    return LCD_OK;
}

LCD_StatusTypeDef LCD_CursorOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->cursorOn = true;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_CursorOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->cursorOn = false;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_BlinkOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->blinkOn = true;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_BlinkOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->blinkOn = false;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_CursorLeft(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, LCD_CMD_CURSOR_SHIFT | LCD_SHIFT_CURSOR | LCD_SHIFT_LEFT, 0);

    if (handle->cursorCol > 0U)
    {
        handle->cursorCol--;
    }

    return LCD_OK;
}

LCD_StatusTypeDef LCD_CursorRight(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, LCD_CMD_CURSOR_SHIFT | LCD_SHIFT_CURSOR | LCD_SHIFT_RIGHT, 0);

    if ((handle->cursorCol + 1U) < handle->cols)
    {
        handle->cursorCol++;
    }

    return LCD_OK;
}
