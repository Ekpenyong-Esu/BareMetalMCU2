/**
 * @file    lcd_types.h
 * @brief   Character LCD data types and HD44780 constants
 * @details Shared by every LCD module. Contains no behaviour.
 */

#ifndef LCD_TYPES_H
#define LCD_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported types ------------------------------------------------------------*/

/**
 * @brief LCD Status enumeration
 */
typedef enum {
    LCD_OK = 0,         /**< Operation completed successfully */
    LCD_ERROR,          /**< General error occurred */
    LCD_BUSY,           /**< LCD is busy */
    LCD_TIMEOUT,        /**< Operation timed out */
    LCD_INVALID_PARAM,  /**< Invalid parameter provided */
    LCD_NOT_INITIALIZED /**< Driver not initialized */
} LCD_StatusTypeDef;

/**
 * @brief LCD interface mode
 */
typedef enum {
    LCD_MODE_4BIT = 0, /**< 4-bit data interface */
    LCD_MODE_8BIT      /**< 8-bit data interface */
} LCD_ModeTypeDef;

/**
 * @brief LCD size type
 */
typedef enum {
    LCD_SIZE_16x2 = 0, /**< 16 columns x 2 rows */
    LCD_SIZE_20x4,     /**< 20 columns x 4 rows */
    LCD_SIZE_16x4,     /**< 16 columns x 4 rows */
    LCD_SIZE_20x2,     /**< 20 columns x 2 rows */
    LCD_SIZE_24x2,     /**< 24 columns x 2 rows */
    LCD_SIZE_8x2,      /**< 8 columns x 2 rows */
    LCD_SIZE_40x2,     /**< 40 columns x 2 rows */
    LCD_SIZE_CUSTOM    /**< Custom size */
} LCD_SizeTypeDef;

/**
 * @brief LCD GPIO pin configuration
 */
typedef struct {
    GPIO_TypeDef *port; /**< GPIO port */
    uint16_t pin;       /**< GPIO pin */
} LCD_PinTypeDef;

/**
 * @brief LCD pin configuration structure
 */
typedef struct {
    LCD_PinTypeDef rs;        /**< Register Select pin */
    LCD_PinTypeDef rw;        /**< Read/Write pin (optional, can be tied to GND) */
    LCD_PinTypeDef en;        /**< Enable pin */
    LCD_PinTypeDef d0;        /**< Data pin 0 (8-bit mode only) */
    LCD_PinTypeDef d1;        /**< Data pin 1 (8-bit mode only) */
    LCD_PinTypeDef d2;        /**< Data pin 2 (8-bit mode only) */
    LCD_PinTypeDef d3;        /**< Data pin 3 (8-bit mode only) */
    LCD_PinTypeDef d4;        /**< Data pin 4 */
    LCD_PinTypeDef d5;        /**< Data pin 5 */
    LCD_PinTypeDef d6;        /**< Data pin 6 */
    LCD_PinTypeDef d7;        /**< Data pin 7 */
    LCD_PinTypeDef backlight; /**< Backlight control pin (optional) */
} LCD_PinsTypeDef;

/**
 * @brief LCD configuration structure
 */
typedef struct {
    LCD_PinsTypeDef pins; /**< GPIO pin assignments */
    LCD_ModeTypeDef mode; /**< 4-bit or 8-bit mode */
    LCD_SizeTypeDef size; /**< Display size preset */
    uint8_t cols;         /**< Number of columns (for custom size) */
    uint8_t rows;         /**< Number of rows (for custom size) */
    bool useRW;           /**< Use R/W pin (false = tied to GND) */
    bool useBacklight;    /**< Use backlight control pin */
} LCD_ConfigTypeDef;

/**
 * @brief LCD handle structure
 */
typedef struct {
    LCD_ConfigTypeDef config; /**< LCD configuration */
    uint8_t cols;             /**< Number of columns */
    uint8_t rows;             /**< Number of rows */
    uint8_t cursorCol;        /**< Current cursor column */
    uint8_t cursorRow;        /**< Current cursor row */
    bool displayOn;           /**< Display on/off state */
    bool cursorOn;            /**< Cursor on/off state */
    bool blinkOn;             /**< Cursor blink on/off state */
    bool initialized;         /**< Initialization flag */
} LCD_HandleTypeDef;

/* Exported constants --------------------------------------------------------*/

/** @defgroup LCD_Commands HD44780 Command Definitions
 * @{
 */
#define LCD_CMD_CLEAR_DISPLAY 0x01U   /**< Clear display */
#define LCD_CMD_RETURN_HOME 0x02U     /**< Return cursor to home */
#define LCD_CMD_ENTRY_MODE_SET 0x04U  /**< Entry mode set */
#define LCD_CMD_DISPLAY_CONTROL 0x08U /**< Display on/off control */
#define LCD_CMD_CURSOR_SHIFT 0x10U    /**< Cursor/display shift */
#define LCD_CMD_FUNCTION_SET 0x20U    /**< Function set */
#define LCD_CMD_SET_CGRAM_ADDR 0x40U  /**< Set CGRAM address */
#define LCD_CMD_SET_DDRAM_ADDR 0x80U  /**< Set DDRAM address */

/** @} */

/** @defgroup LCD_EntryMode Entry Mode Flags
 * @{
 */
#define LCD_ENTRY_RIGHT 0x00U     /**< Decrement cursor position */
#define LCD_ENTRY_LEFT 0x02U      /**< Increment cursor position */
#define LCD_ENTRY_SHIFT_ON 0x01U  /**< Shift display on write */
#define LCD_ENTRY_SHIFT_OFF 0x00U /**< No display shift */

/** @} */

/** @defgroup LCD_DisplayControl Display Control Flags
 * @{
 */
#define LCD_DISPLAY_ON 0x04U  /**< Display on */
#define LCD_DISPLAY_OFF 0x00U /**< Display off */
#define LCD_CURSOR_ON 0x02U   /**< Cursor on */
#define LCD_CURSOR_OFF 0x00U  /**< Cursor off */
#define LCD_BLINK_ON 0x01U    /**< Cursor blink on */
#define LCD_BLINK_OFF 0x00U   /**< Cursor blink off */

/** @} */

/** @defgroup LCD_CursorShift Cursor/Display Shift Flags
 * @{
 */
#define LCD_SHIFT_CURSOR 0x00U  /**< Move the cursor */
#define LCD_SHIFT_DISPLAY 0x08U /**< Shift the whole display */
#define LCD_SHIFT_LEFT 0x00U    /**< Shift towards the left */
#define LCD_SHIFT_RIGHT 0x04U   /**< Shift towards the right */

/** @} */

/** @defgroup LCD_FunctionSet Function Set Flags
 * @{
 */
#define LCD_8BIT_MODE 0x10U /**< 8-bit data interface */
#define LCD_4BIT_MODE 0x00U /**< 4-bit data interface */
#define LCD_2LINE 0x08U     /**< 2-line display */
#define LCD_1LINE 0x00U     /**< 1-line display */
#define LCD_5x10_DOTS 0x04U /**< 5x10 dot character font */
#define LCD_5x8_DOTS 0x00U  /**< 5x8 dot character font */

/** @} */

/** @defgroup LCD_DDRAM_Addresses DDRAM Row Addresses
 * @{
 */
#define LCD_ROW0_ADDR 0x00U /**< Row 0 start address */
#define LCD_ROW1_ADDR 0x40U /**< Row 1 start address */
#define LCD_ROW2_ADDR 0x14U /**< Row 2 start address (20x4) */
#define LCD_ROW3_ADDR 0x54U /**< Row 3 start address (20x4) */

/** @} */

/** @defgroup LCD_Limits Geometry Limits
 * @{
 */
#define LCD_MAX_ROWS 4U            /**< DDRAM row address table size */
#define LCD_MAX_COLS 40U           /**< Widest HD44780 line */
#define LCD_CGRAM_SLOTS 8U         /**< Number of custom character slots */
#define LCD_CGRAM_PATTERN_BYTES 8U /**< Rows per custom character pattern */

/** @} */

/** @defgroup LCD_Timing Timing Constants
 * @{
 */
#define LCD_ENABLE_PULSE_US 1U      /**< Enable pulse width (us) */
#define LCD_COMMAND_DELAY_US 50U    /**< Command execution delay (us) */
#define LCD_CLEAR_DELAY_MS 2U       /**< Clear/home command delay (ms) */
#define LCD_INIT_DELAY_MS 50U       /**< Power-on initialization delay (ms) */
#define LCD_INIT_NIBBLE_DELAY_MS 5U /**< Delay after the first wake-up nibble (ms) */
#define LCD_INIT_SETTLE_US 150U     /**< Delay between HD44780 wake-up nibbles (us) */
#define LCD_INIT_NIBBLE_8BIT 0x03U  /**< Wake-up nibble: request 8-bit interface */
#define LCD_INIT_NIBBLE_4BIT 0x02U  /**< Switch the interface to 4-bit */

/** @} */

/* Exported macros -----------------------------------------------------------*/

/** Reject a NULL handle */
#define LCD_CHECK_HANDLE(h)                                                                        \
    do {                                                                                           \
        if ((h) == NULL)                                                                           \
            return LCD_INVALID_PARAM;                                                              \
    } while (0)

/** Reject an uninitialised handle */
#define LCD_CHECK_INIT(h)                                                                          \
    do {                                                                                           \
        if (!(h)->initialized)                                                                     \
            return LCD_NOT_INITIALIZED;                                                            \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* LCD_TYPES_H */
