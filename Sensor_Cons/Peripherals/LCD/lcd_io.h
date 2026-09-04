/**
 * @file    lcd_io.h
 * @brief   Low level HD44780 transport: GPIO wiring, timing and bus writes
 * @details Internal to the LCD driver. Application code should use the
 *          lcd_core / lcd_display / lcd_cursor / lcd_print modules instead.
 */

#ifndef LCD_IO_H
#define LCD_IO_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "lcd_types.h"

/* Exported functions --------------------------------------------------------*/

/**
 * @brief   Busy-wait for the given number of microseconds
 * @param   micros Delay in microseconds
 */
void LCD_IO_DelayUs(uint32_t micros);

/**
 * @brief   Block for the given number of milliseconds
 * @param   millis Delay in milliseconds
 */
void LCD_IO_DelayMs(uint32_t millis);

/**
 * @brief   Drive one configured LCD pin, ignoring unwired (NULL port) pins
 * @param   pin   Pin configuration
 * @param   state Pin state
 */
void LCD_IO_SetPin(const LCD_PinTypeDef *pin, GPIO_PinState state);

/**
 * @brief   Configure every pin the handle's configuration uses as an output
 * @param   handle LCD handle
 */
void LCD_IO_InitGPIO(LCD_HandleTypeDef *handle);

/**
 * @brief   Clock a 4-bit nibble into the panel over D4..D7
 * @param   handle LCD handle
 * @param   nibble 4-bit data (lower nibble)
 */
void LCD_IO_WriteNibble(LCD_HandleTypeDef *handle, uint8_t nibble);

/**
 * @brief   Write one byte to the panel in the configured bus width
 * @param   handle    LCD handle
 * @param   byte      Data byte
 * @param   regSelect Register select (0 = command, 1 = data)
 */
void LCD_IO_WriteByte(LCD_HandleTypeDef *handle, uint8_t byte, uint8_t regSelect);

#ifdef __cplusplus
}
#endif

#endif /* LCD_IO_H */
