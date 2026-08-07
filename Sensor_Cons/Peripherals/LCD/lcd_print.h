/**
 * @file    lcd_print.h
 * @brief   Character LCD text output
 */

#ifndef __LCD_PRINT_H__
#define __LCD_PRINT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lcd_types.h"

/* Exported constants --------------------------------------------------------*/

/** Largest number of fractional digits LCD_PrintFloat() will emit */
#define LCD_MAX_DECIMALS        7U

/** Size of the scratch buffer used by LCD_Printf() */
#define LCD_PRINTF_BUFFER_SIZE  64U

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Print a single character at the current cursor position
 * @note    On reaching the end of a row the cursor is re-addressed to the
 *          start of the next row, wrapping back to row 0 after the last one.
 * @param   handle Pointer to LCD handle
 * @param   ch     Character to print
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintChar(LCD_HandleTypeDef* handle, char ch);

/**
 * @brief   Print a NUL terminated string at the current cursor position
 * @param   handle Pointer to LCD handle
 * @param   str    String to print
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintString(LCD_HandleTypeDef* handle, const char* str);

/**
 * @brief   Move the cursor then print a string
 * @param   handle Pointer to LCD handle
 * @param   col    Column index
 * @param   row    Row index
 * @param   str    String to print
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintStringAt(LCD_HandleTypeDef* handle, uint8_t col,
                                    uint8_t row, const char* str);

/**
 * @brief   Print a signed decimal integer
 * @param   handle Pointer to LCD handle
 * @param   value  Value to print
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintInt(LCD_HandleTypeDef* handle, int32_t value);

/**
 * @brief   Print a floating-point value in fixed notation
 * @param   handle   Pointer to LCD handle
 * @param   value    Value to print
 * @param   decimals Fractional digits, clamped to LCD_MAX_DECIMALS
 * @retval  LCD_StatusTypeDef Operation status, LCD_INVALID_PARAM when the
 *          value is not finite or too large for a 32-bit integer part
 */
LCD_StatusTypeDef LCD_PrintFloat(LCD_HandleTypeDef* handle, float value, uint8_t decimals);

/**
 * @brief   Print an unsigned value as "0x" followed by upper-case hex digits
 * @param   handle Pointer to LCD handle
 * @param   value  Value to print
 * @param   digits Number of digits, clamped to 1..8
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_PrintHex(LCD_HandleTypeDef* handle, uint32_t value, uint8_t digits);

/**
 * @brief   Print a printf-style formatted string
 * @note    Output longer than LCD_PRINTF_BUFFER_SIZE - 1 characters is truncated.
 * @param   handle Pointer to LCD handle
 * @param   format Format string
 * @retval  LCD_StatusTypeDef Operation status
 */
LCD_StatusTypeDef LCD_Printf(LCD_HandleTypeDef* handle, const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif /* __LCD_PRINT_H__ */
