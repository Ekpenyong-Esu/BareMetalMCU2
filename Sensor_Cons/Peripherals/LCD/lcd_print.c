/**
 * @file    lcd_print.c
 * @brief   Character LCD text output
 */

/* Includes ------------------------------------------------------------------*/
#include "lcd_print.h"
#include "lcd_cursor.h"
#include "lcd_io.h"
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/* Private define ------------------------------------------------------------*/

/** Radix of the decimal renderers */
#define LCD_DECIMAL_BASE 10U

/** Digits in the largest 32-bit unsigned value (4294967295) */
#define LCD_MAX_DECIMAL_DIGITS 10U

/** 2^32: the smallest float whose integer part no longer fits a uint32_t */
#define LCD_UINT32_RANGE_F 4294967296.0f

/** Digits in the largest 32-bit value, plus sign and terminator */
#define LCD_INT_BUFFER_SIZE 12U

/** "0x" + 8 hex digits + terminator */
#define LCD_HEX_BUFFER_SIZE 11U

/** Sign + 10 integer digits + point + LCD_MAX_DECIMALS digits + terminator */
#define LCD_FLOAT_BUFFER_SIZE (13U + LCD_MAX_DECIMALS)

/** Largest number of hex digits LCD_PrintHex() will emit */
#define LCD_MAX_HEX_DIGITS 8U

/* Private function prototypes -----------------------------------------------*/
static uint8_t LCD_FormatUnsigned(uint32_t value, char *buffer);

/* Public functions ----------------------------------------------------------*/

LCD_StatusTypeDef LCD_PrintChar(LCD_HandleTypeDef *handle, char character) {
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, (uint8_t)character, 1);

    handle->cursorCol++;

    if (handle->cursorCol < handle->cols) {
        return LCD_OK;
    }

    /* HD44780 DDRAM addresses are not contiguous across rows, so the address
       counter has to be moved explicitly rather than left to run on. */
    handle->cursorCol = 0;
    handle->cursorRow++;
    if (handle->cursorRow >= handle->rows) {
        handle->cursorRow = 0;
    }

    return LCD_SetCursor(handle, handle->cursorCol, handle->cursorRow);
}

LCD_StatusTypeDef LCD_PrintString(LCD_HandleTypeDef *handle, const char *str) {
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (str == NULL) {
        return LCD_INVALID_PARAM;
    }

    while (*str != '\0') {
        const LCD_StatusTypeDef status = LCD_PrintChar(handle, *str);

        if (status != LCD_OK) {
            return status;
        }

        str++;
    }

    return LCD_OK;
}

LCD_StatusTypeDef LCD_PrintStringAt(LCD_HandleTypeDef *handle, uint8_t col, uint8_t row,
                                    const char *str) {
    const LCD_StatusTypeDef status = LCD_SetCursor(handle, col, row);

    if (status != LCD_OK) {
        return status;
    }

    return LCD_PrintString(handle, str);
}

LCD_StatusTypeDef LCD_PrintInt(LCD_HandleTypeDef *handle, int32_t value) {
    char buffer[LCD_INT_BUFFER_SIZE] = {0};
    uint8_t idx = 0;
    uint32_t magnitude = 0;

    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (value < 0) {
        buffer[idx++] = '-';
        /* Negating INT32_MIN directly overflows, so fold it via its predecessor */
        magnitude = (uint32_t)(-(value + 1)) + 1U;
    }
    else {
        magnitude = (uint32_t)value;
    }

    idx = (uint8_t)(idx + LCD_FormatUnsigned(magnitude, &buffer[idx]));
    buffer[idx] = '\0';

    return LCD_PrintString(handle, buffer);
}

LCD_StatusTypeDef LCD_PrintFloat(LCD_HandleTypeDef *handle, float value, uint8_t decimals) {
    char buffer[LCD_FLOAT_BUFFER_SIZE] = {0};
    uint8_t idx = 0;
    uint32_t intPart = 0;
    float fracPart = NAN;

    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (!isfinite(value)) {
        return LCD_INVALID_PARAM;
    }

    if (decimals > LCD_MAX_DECIMALS) {
        decimals = LCD_MAX_DECIMALS;
    }

    if (value < 0.0f) {
        buffer[idx++] = '-';
        value = -value;
    }

    /* Casting a float larger than UINT32_MAX to an integer is undefined */
    if (value >= LCD_UINT32_RANGE_F) {
        return LCD_INVALID_PARAM;
    }

    intPart = (uint32_t)value;
    fracPart = value - (float)intPart;

    idx = (uint8_t)(idx + LCD_FormatUnsigned(intPart, &buffer[idx]));

    if (decimals > 0U) {
        buffer[idx++] = '.';

        for (uint8_t i = 0; i < decimals; i++) {
            uint8_t digit = 0;

            fracPart *= (float)LCD_DECIMAL_BASE;
            digit = (uint8_t)fracPart;
            buffer[idx++] = (char)('0' + digit);
            fracPart -= (float)digit;
        }
    }

    buffer[idx] = '\0';

    return LCD_PrintString(handle, buffer);
}

LCD_StatusTypeDef LCD_PrintHex(LCD_HandleTypeDef *handle, uint32_t value, uint8_t digits) {
    static const char hexChars[] = "0123456789ABCDEF";
    char buffer[LCD_HEX_BUFFER_SIZE] = {0};
    uint8_t idx = 0;

    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (digits > LCD_MAX_HEX_DIGITS) {
        digits = LCD_MAX_HEX_DIGITS;
    }
    if (digits == 0U) {
        digits = 1U;
    }

    buffer[0] = '0';
    buffer[1] = 'x';

    for (idx = 0; idx < digits; idx++) {
        const uint8_t nibble = (uint8_t)((value >> (4U * (digits - 1U - idx))) & 0x0FU);

        buffer[2U + idx] = hexChars[nibble];
    }
    buffer[2U + digits] = '\0';

    return LCD_PrintString(handle, buffer);
}

LCD_StatusTypeDef LCD_Printf(LCD_HandleTypeDef *handle, const char *format, ...) {
    char buffer[LCD_PRINTF_BUFFER_SIZE] = {0};
    va_list args;

    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    if (format == NULL) {
        return LCD_INVALID_PARAM;
    }

    va_start(args, format);
    (void)vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    return LCD_PrintString(handle, buffer);
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Render an unsigned value as decimal digits, without a terminator
 * @param   value  Value to render
 * @param   buffer Destination, must hold at least LCD_MAX_DECIMAL_DIGITS characters
 * @retval  Number of characters written
 */
static uint8_t LCD_FormatUnsigned(uint32_t value, char *buffer) {
    char digits[LCD_MAX_DECIMAL_DIGITS];
    uint8_t count = 0;
    uint8_t written = 0;

    if (value == 0U) {
        buffer[0] = '0';
        return 1U;
    }

    while (value > 0U) {
        digits[count++] = (char)('0' + (value % LCD_DECIMAL_BASE));
        value /= LCD_DECIMAL_BASE;
    }

    while (count > 0U) {
        count--;
        buffer[written++] = digits[count];
    }

    return written;
}
