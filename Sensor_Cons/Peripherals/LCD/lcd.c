/**
 * @file    lcd.c
 * @brief   Character LCD lifecycle, geometry and raw bus access
 */

/* Includes ------------------------------------------------------------------*/
#include "lcd_core.h"
#include "lcd_io.h"
#include "lcd_display.h"

/* Private define ------------------------------------------------------------*/

/** Column and row counts of the preset panel sizes */
#define LCD_COLS_8 8U
#define LCD_COLS_16 16U
#define LCD_COLS_20 20U
#define LCD_COLS_24 24U
#define LCD_COLS_40 40U
#define LCD_ROWS_2 2U
#define LCD_ROWS_4 4U

/* Private variables ---------------------------------------------------------*/

/** DDRAM start address of each row, one table per physical layout */
static const uint8_t rowOffsets_16x2[LCD_MAX_ROWS] = {0x00, 0x40, 0x00, 0x40};
static const uint8_t rowOffsets_20x4[LCD_MAX_ROWS] = {0x00, 0x40, 0x14, 0x54};
static const uint8_t rowOffsets_16x4[LCD_MAX_ROWS] = {0x00, 0x40, 0x10, 0x50};

/* Private function prototypes -----------------------------------------------*/
static LCD_StatusTypeDef LCD_ValidatePins(const LCD_ConfigTypeDef *config);
static LCD_StatusTypeDef LCD_ResolveGeometry(const LCD_ConfigTypeDef *config, uint8_t *cols,
                                             uint8_t *rows);
static void LCD_GetDimensions(LCD_SizeTypeDef size, uint8_t *cols, uint8_t *rows);
static void LCD_SendResetSequence(LCD_HandleTypeDef *handle);

/* Public functions ----------------------------------------------------------*/

LCD_StatusTypeDef LCD_Init(LCD_HandleTypeDef *handle, const LCD_ConfigTypeDef *config) {
    LCD_StatusTypeDef status = LCD_OK;

    LCD_CHECK_HANDLE(handle);

    if (config == NULL) {
        return LCD_INVALID_PARAM;
    }

    status = LCD_ValidatePins(config);
    if (status != LCD_OK) {
        return status;
    }

    handle->config = *config;

    status = LCD_ResolveGeometry(config, &handle->cols, &handle->rows);
    if (status != LCD_OK) {
        return status;
    }

    handle->cursorCol = 0;
    handle->cursorRow = 0;
    handle->displayOn = true;
    handle->cursorOn = false;
    handle->blinkOn = false;
    handle->initialized = false;

    LCD_IO_InitGPIO(handle);

    LCD_IO_DelayMs(LCD_INIT_DELAY_MS);

    LCD_IO_SetPin(&handle->config.pins.rs, GPIO_PIN_RESET);
    if (handle->config.useRW) {
        LCD_IO_SetPin(&handle->config.pins.rw, GPIO_PIN_RESET);
    }

    LCD_SendResetSequence(handle);

    /* Display off while the entry mode is still unknown */
    LCD_IO_WriteByte(handle, LCD_CMD_DISPLAY_CONTROL, 0);

    LCD_IO_WriteByte(handle, LCD_CMD_CLEAR_DISPLAY, 0);
    LCD_IO_DelayMs(LCD_CLEAR_DELAY_MS);

    LCD_IO_WriteByte(handle, LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_LEFT | LCD_ENTRY_SHIFT_OFF, 0);

    LCD_UpdateDisplayControl(handle);

    handle->initialized = true;

    return LCD_OK;
}

LCD_StatusTypeDef LCD_InitDefault(LCD_HandleTypeDef *handle, const LCD_PinsTypeDef *pins) {
    LCD_ConfigTypeDef config = {0};

    LCD_CHECK_HANDLE(handle);

    if (pins == NULL) {
        return LCD_INVALID_PARAM;
    }

    config.pins = *pins;
    config.mode = LCD_MODE_4BIT;
    config.size = LCD_SIZE_16x2;
    config.useRW = false;
    config.useBacklight = (pins->backlight.port != NULL);

    return LCD_Init(handle, &config);
}

LCD_StatusTypeDef LCD_DeInit(LCD_HandleTypeDef *handle) {
    LCD_CHECK_HANDLE(handle);

    if (handle->initialized) {
        (void)LCD_DisplayOff(handle);
        (void)LCD_BacklightOff(handle);
    }

    handle->initialized = false;

    return LCD_OK;
}

LCD_StatusTypeDef LCD_SendCommand(LCD_HandleTypeDef *handle, uint8_t cmd) {
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, cmd, 0);

    return LCD_OK;
}

LCD_StatusTypeDef LCD_SendData(LCD_HandleTypeDef *handle, uint8_t data) {
    LCD_CHECK_HANDLE(handle);
    LCD_CHECK_INIT(handle);

    LCD_IO_WriteByte(handle, data, 1);

    return LCD_OK;
}

void LCD_UpdateDisplayControl(LCD_HandleTypeDef *handle) {
    uint8_t cmd = LCD_CMD_DISPLAY_CONTROL;

    if (handle->displayOn) {
        cmd |= LCD_DISPLAY_ON;
    }
    if (handle->cursorOn) {
        cmd |= LCD_CURSOR_ON;
    }
    if (handle->blinkOn) {
        cmd |= LCD_BLINK_ON;
    }

    LCD_IO_WriteByte(handle, cmd, 0);
}

const uint8_t *LCD_GetRowOffsets(LCD_SizeTypeDef size) {
    switch (size) {
        case LCD_SIZE_20x4:
            return rowOffsets_20x4;
        case LCD_SIZE_16x4:
            return rowOffsets_16x4;
        default:
            return rowOffsets_16x2;
    }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Reject a configuration whose mandatory pins are unwired
 * @note    LCD_IO_SetPin() silently ignores a NULL port, so without this check
 *          an unwired configuration would "initialize" and never drive the panel.
 * @param   config Configuration to validate
 * @retval  LCD_StatusTypeDef Operation status
 */
static LCD_StatusTypeDef LCD_ValidatePins(const LCD_ConfigTypeDef *config) {
    if (config->pins.rs.port == NULL || config->pins.en.port == NULL ||
        config->pins.d4.port == NULL || config->pins.d5.port == NULL ||
        config->pins.d6.port == NULL || config->pins.d7.port == NULL) {
        return LCD_INVALID_PARAM;
    }

    if (config->mode == LCD_MODE_8BIT &&
        (config->pins.d0.port == NULL || config->pins.d1.port == NULL ||
         config->pins.d2.port == NULL || config->pins.d3.port == NULL)) {
        return LCD_INVALID_PARAM;
    }

    if (config->useRW && config->pins.rw.port == NULL) {
        return LCD_INVALID_PARAM;
    }

    if (config->useBacklight && config->pins.backlight.port == NULL) {
        return LCD_INVALID_PARAM;
    }

    return LCD_OK;
}

/**
 * @brief   Work out the column and row count a configuration asks for
 * @note    The row count is bounded by LCD_MAX_ROWS because the DDRAM offset
 *          tables have that many entries and LCD_SetCursor() indexes them.
 * @param   config Configuration to read
 * @param   cols   Receives the column count
 * @param   rows   Receives the row count
 * @retval  LCD_StatusTypeDef Operation status
 */
static LCD_StatusTypeDef LCD_ResolveGeometry(const LCD_ConfigTypeDef *config, uint8_t *cols,
                                             uint8_t *rows) {
    if (config->size == LCD_SIZE_CUSTOM) {
        *cols = config->cols;
        *rows = config->rows;
    }
    else {
        LCD_GetDimensions(config->size, cols, rows);
    }

    if (*cols == 0U || *cols > LCD_MAX_COLS || *rows == 0U || *rows > LCD_MAX_ROWS) {
        return LCD_INVALID_PARAM;
    }

    return LCD_OK;
}

/**
 * @brief   Column and row count of a preset display size
 * @param   size Display size type
 * @param   cols Receives the column count
 * @param   rows Receives the row count
 */
static void LCD_GetDimensions(LCD_SizeTypeDef size, uint8_t *cols, uint8_t *rows) {
    switch (size) {
        case LCD_SIZE_20x4:
            *cols = LCD_COLS_20;
            *rows = LCD_ROWS_4;
            break;
        case LCD_SIZE_16x4:
            *cols = LCD_COLS_16;
            *rows = LCD_ROWS_4;
            break;
        case LCD_SIZE_20x2:
            *cols = LCD_COLS_20;
            *rows = LCD_ROWS_2;
            break;
        case LCD_SIZE_24x2:
            *cols = LCD_COLS_24;
            *rows = LCD_ROWS_2;
            break;
        case LCD_SIZE_8x2:
            *cols = LCD_COLS_8;
            *rows = LCD_ROWS_2;
            break;
        case LCD_SIZE_40x2:
            *cols = LCD_COLS_40;
            *rows = LCD_ROWS_2;
            break;
        case LCD_SIZE_16x2:
        default:
            *cols = LCD_COLS_16;
            *rows = LCD_ROWS_2;
            break;
    }
}

/**
 * @brief   Run the HD44780 power-on sequence for the configured bus width
 * @note    The controller may be in either bus width after a warm reset, so the
 *          8-bit function set is issued three times before the width is fixed.
 * @param   handle LCD handle
 */
static void LCD_SendResetSequence(LCD_HandleTypeDef *handle) {
    if (handle->config.mode == LCD_MODE_4BIT) {
        LCD_IO_WriteNibble(handle, LCD_INIT_NIBBLE_8BIT);
        LCD_IO_DelayMs(LCD_INIT_NIBBLE_DELAY_MS);

        LCD_IO_WriteNibble(handle, LCD_INIT_NIBBLE_8BIT);
        LCD_IO_DelayUs(LCD_INIT_SETTLE_US);

        LCD_IO_WriteNibble(handle, LCD_INIT_NIBBLE_8BIT);
        LCD_IO_DelayUs(LCD_INIT_SETTLE_US);

        LCD_IO_WriteNibble(handle, LCD_INIT_NIBBLE_4BIT);
        LCD_IO_DelayUs(LCD_INIT_SETTLE_US);

        LCD_IO_WriteByte(handle, LCD_CMD_FUNCTION_SET | LCD_4BIT_MODE | LCD_2LINE | LCD_5x8_DOTS,
                         0);
    }
    else {
        const uint8_t functionSet = LCD_CMD_FUNCTION_SET | LCD_8BIT_MODE | LCD_2LINE | LCD_5x8_DOTS;

        LCD_IO_WriteByte(handle, functionSet, 0);
        LCD_IO_DelayMs(LCD_INIT_NIBBLE_DELAY_MS);

        LCD_IO_WriteByte(handle, functionSet, 0);
        LCD_IO_DelayUs(LCD_INIT_SETTLE_US);

        LCD_IO_WriteByte(handle, functionSet, 0);
    }
}
