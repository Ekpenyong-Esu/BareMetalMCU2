/**
 * @file    lcd_display.c
 * @brief   Character LCD display level control: clearing, power and scrolling
 */

/* Includes ------------------------------------------------------------------*/
#include "lcd_display.h"
#include "lcd_core.h"
#include "lcd_cursor.h"
#include "lcd_print.h"
#include "lcd_io.h"

/* Public functions ----------------------------------------------------------*/

LCD_StatusTypeDef LCD_Clear(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, LCD_CMD_CLEAR_DISPLAY, 0);
    LCD_IO_DelayMs(LCD_CLEAR_DELAY_MS);

    handle->cursorCol = 0;
    handle->cursorRow = 0;

    return LCD_OK;
}

LCD_StatusTypeDef LCD_Home(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, LCD_CMD_RETURN_HOME, 0);
    LCD_IO_DelayMs(LCD_CLEAR_DELAY_MS);

    handle->cursorCol = 0;
    handle->cursorRow = 0;

    return LCD_OK;
}

LCD_StatusTypeDef LCD_DisplayOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->displayOn = true;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_DisplayOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    handle->displayOn = false;
    LCD_UpdateDisplayControl(handle);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_BacklightOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (handle->config.useBacklight)
    {
        LCD_IO_SetPin(&handle->config.pins.backlight, GPIO_PIN_SET);
    }

    return LCD_OK;
}

LCD_StatusTypeDef LCD_BacklightOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (handle->config.useBacklight)
    {
        LCD_IO_SetPin(&handle->config.pins.backlight, GPIO_PIN_RESET);
    }

    return LCD_OK;
}

LCD_StatusTypeDef LCD_ClearLine(LCD_HandleTypeDef* handle, uint8_t row)
{
    LCD_StatusTypeDef status;

    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (row >= handle->rows)
    {
        return LCD_INVALID_PARAM;
    }

    status = LCD_SetCursor(handle, 0, row);
    if (status != LCD_OK)
    {
        return status;
    }

    for (uint8_t i = 0; i < handle->cols; i++)
    {
        status = LCD_PrintChar(handle, ' ');
        if (status != LCD_OK)
        {
            return status;
        }
    }

    return LCD_SetCursor(handle, 0, row);
}

LCD_StatusTypeDef LCD_ScrollLeft(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, LCD_CMD_CURSOR_SHIFT | LCD_SHIFT_DISPLAY | LCD_SHIFT_LEFT, 0);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_ScrollRight(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, LCD_CMD_CURSOR_SHIFT | LCD_SHIFT_DISPLAY | LCD_SHIFT_RIGHT, 0);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_AutoScrollOn(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_ON, 0);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_AutoScrollOff(LCD_HandleTypeDef* handle)
{
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_OFF, 0);

    return LCD_OK;
}
