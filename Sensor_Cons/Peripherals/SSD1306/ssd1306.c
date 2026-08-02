/**
  ******************************************************************************
  * @file    ssd1306.c
  * @brief   SSD1306 OLED Display Driver Implementation
  * @details This file provides the implementation of SSD1306 OLED functions
  *          using I2C interface on STM32F429I-DISC1.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ssd1306.h"
#include "i2c.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Private variables ---------------------------------------------------------*/

/** @defgroup SSD1306_Private_Variables Private Variables
 * @{
 */

/* Global display buffer - shared across all SSD1306 instances */
uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

/* Static buffer for I2C transmissions - avoids dynamic allocation */
static uint8_t ssd1306_tx_buffer[SSD1306_BUFFER_SIZE + 1];

/* Global SSD1306 handle for interrupt handling */
static SSD1306_Handle_t *g_hssd = NULL;

/** @} */

/* Private defines -----------------------------------------------------------*/

/** @defgroup SSD1306_Private_Defines Private Defines
 * @{
 */

/* SSD1306 Commands */
#define SSD1306_CMD_SET_CONTRAST        0x81
#define SSD1306_CMD_DISPLAY_ALL_ON_RESUME 0xA4
#define SSD1306_CMD_DISPLAY_ALL_ON       0xA5
#define SSD1306_CMD_NORMAL_DISPLAY       0xA6
#define SSD1306_CMD_INVERT_DISPLAY       0xA7
#define SSD1306_CMD_DISPLAY_OFF          0xAE
#define SSD1306_CMD_DISPLAY_ON           0xAF
#define SSD1306_CMD_SET_DISPLAY_OFFSET   0xD3
#define SSD1306_CMD_SET_COM_PINS         0xDA
#define SSD1306_CMD_SET_VCOM_DETECT      0xDB
#define SSD1306_CMD_SET_DISPLAY_CLOCK_DIV 0xD5
#define SSD1306_CMD_SET_PRECHARGE        0xD9
#define SSD1306_CMD_SET_MULTIPLEX        0xA8
#define SSD1306_CMD_SET_LOW_COLUMN       0x00
#define SSD1306_CMD_SET_HIGH_COLUMN      0x10
#define SSD1306_CMD_SET_START_LINE       0x40
#define SSD1306_CMD_MEMORY_MODE          0x20
#define SSD1306_CMD_COLUMN_ADDR          0x21
#define SSD1306_CMD_PAGE_ADDR            0x22
#define SSD1306_CMD_COM_SCAN_INC         0xC0
#define SSD1306_CMD_COM_SCAN_DEC         0xC8
#define SSD1306_CMD_SEG_REMAP            0xA0
#define SSD1306_CMD_CHARGE_PUMP          0x8D

/* Font data: printable ASCII, 6 column bytes per glyph, bit N = row N. */
#define SSD1306_FONT_FIRST_CHAR 32
#define SSD1306_FONT_LAST_CHAR  127

static const uint8_t font6x8[96][6] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62, 0x00}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50, 0x00}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14, 0x00}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08, 0x00}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08, 0x00}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02, 0x00}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46, 0x00}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31, 0x00}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10, 0x00}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39, 0x00}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03, 0x00}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36, 0x00}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E, 0x00}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14, 0x00}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08, 0x00}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06, 0x00}, // ?
    {0x32, 0x49, 0x59, 0x51, 0x3E, 0x00}, // @
    {0x7C, 0x12, 0x11, 0x12, 0x7C, 0x00}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36, 0x00}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22, 0x00}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C, 0x00}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41, 0x00}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01, 0x00}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A, 0x00}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01, 0x00}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41, 0x00}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40, 0x00}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06, 0x00}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46, 0x00}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31, 0x00}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01, 0x00}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63, 0x00}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07, 0x00}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43, 0x00}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20, 0x00}, // backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04, 0x00}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40, 0x00}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78, 0x00}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38, 0x00}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20, 0x00}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F, 0x00}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18, 0x00}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02, 0x00}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E, 0x00}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78, 0x00}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78, 0x00}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78, 0x00}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38, 0x00}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08, 0x00}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C, 0x00}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08, 0x00}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20, 0x00}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20, 0x00}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44, 0x00}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44, 0x00}, // z
    {0x00, 0x08, 0x36, 0x41, 0x00, 0x00}, // {
    {0x00, 0x00, 0x7F, 0x00, 0x00, 0x00}, // |
    {0x00, 0x41, 0x36, 0x08, 0x00, 0x00}, // }
    {0x10, 0x08, 0x08, 0x10, 0x08, 0x00}, // ~
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}  // DEL
};

/* Private variables ---------------------------------------------------------*/

/** @defgroup SSD1306_Private_Variables Private Variables
 * @{
 */



/** @} */

/* Private function prototypes -----------------------------------------------*/
static SSD1306_StatusTypeDef SSD1306_WriteCommand(SSD1306_Handle_t *hssd, uint8_t command);
static SSD1306_StatusTypeDef SSD1306_WriteData(SSD1306_Handle_t *hssd, uint8_t *data, uint16_t size);

/* Exported functions -------------------------------------------------------*/

/**
 * @brief   Initialize SSD1306 OLED display
 * @details Configures I2C and initializes the display
 * @param   hssd Pointer to SSD1306 handle
 * @param   address I2C address (0x3C or 0x3D)
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Init(SSD1306_Handle_t *hssd, uint8_t address)
{
    if (hssd == NULL) {
        log_error("SSD1306: Invalid handle pointer");
        return SSD1306_INVALID_PARAM;
    }

    log_info("SSD1306: Initializing display at I2C address 0x%02X", address);

    /* Initialize structure */
    memset(hssd, 0, sizeof(SSD1306_Handle_t));
    hssd->config.address = address;
    g_hssd = hssd;

    /* Initialize display */
    SSD1306_StatusTypeDef status = SSD1306_OK;

    // Display off
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_DISPLAY_OFF);
    if (status != SSD1306_OK) return status;

    // Set display clock divide ratio/oscillator frequency
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_DISPLAY_CLOCK_DIV);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x80);
    if (status != SSD1306_OK) return status;

    // Set multiplex ratio
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_MULTIPLEX);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x3F);
    if (status != SSD1306_OK) return status;

    // Set display offset
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_DISPLAY_OFFSET);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x00);
    if (status != SSD1306_OK) return status;

    // Set start line
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_START_LINE | 0x00);
    if (status != SSD1306_OK) return status;

    // Set charge pump
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_CHARGE_PUMP);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x14);
    if (status != SSD1306_OK) return status;

    // Set memory mode
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_MEMORY_MODE);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x00);
    if (status != SSD1306_OK) return status;

    // Set segment re-map
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SEG_REMAP | 0x01);
    if (status != SSD1306_OK) return status;

    // Set COM output scan direction
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_COM_SCAN_DEC);
    if (status != SSD1306_OK) return status;

    // Set COM pins hardware configuration
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_COM_PINS);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x12);
    if (status != SSD1306_OK) return status;

    // Set contrast control
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_CONTRAST);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0xCF);
    if (status != SSD1306_OK) return status;

    // Set pre-charge period
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_PRECHARGE);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0xF1);
    if (status != SSD1306_OK) return status;

    // Set VCOMH deselect level
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_SET_VCOM_DETECT);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0x40);
    if (status != SSD1306_OK) return status;

    // Set entire display on/off
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_DISPLAY_ALL_ON_RESUME);
    if (status != SSD1306_OK) return status;

    // Set normal display
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_NORMAL_DISPLAY);
    if (status != SSD1306_OK) return status;

    // Clear buffer
    SSD1306_Clear(hssd);

    // Display on
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_DISPLAY_ON);
    if (status != SSD1306_OK) return status;

    hssd->initialized = true;

    log_info("SSD1306: Display initialized successfully");
    return SSD1306_OK;
}

/**
 * @brief   Update display with buffer content
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_UpdateScreen(SSD1306_Handle_t *hssd)
{
    if (hssd == NULL || !hssd->initialized) {
        log_error("SSD1306: Display not initialized");
        return SSD1306_NOT_INITIALIZED;
    }

    log_debug("SSD1306: Updating screen with buffer data");
    SSD1306_StatusTypeDef status;

    // Set column address
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_COLUMN_ADDR);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, SSD1306_WIDTH - 1);
    if (status != SSD1306_OK) return status;

    // Set page address
    status = SSD1306_WriteCommand(hssd, SSD1306_CMD_PAGE_ADDR);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, 0);
    if (status != SSD1306_OK) return status;
    status = SSD1306_WriteCommand(hssd, (SSD1306_HEIGHT / 8) - 1);
    if (status != SSD1306_OK) return status;

    // Write display buffer
    status = SSD1306_WriteData(hssd, SSD1306_Buffer, SSD1306_BUFFER_SIZE);
    if (status == SSD1306_OK) {
        log_debug("SSD1306: Screen updated successfully");
    }

    return status;
}

/**
 * @brief   Clear display buffer
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Clear(SSD1306_Handle_t *hssd)
{
    if (hssd == NULL) {
        log_error("SSD1306: Invalid handle pointer for clear operation");
        return SSD1306_INVALID_PARAM;
    }

    memset(SSD1306_Buffer, 0, SSD1306_BUFFER_SIZE);
    hssd->currentX = 0;
    hssd->currentY = 0;

    log_debug("SSD1306: Display buffer cleared");
    return SSD1306_OK;
}

/**
 * @brief   Fill display buffer with color
 * @param   hssd Pointer to SSD1306 handle
 * @param   color Fill color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Fill(SSD1306_Handle_t *hssd, SSD1306_Color_t color)
{
    if (hssd == NULL) {
        log_error("SSD1306: Invalid handle pointer for fill operation");
        return SSD1306_INVALID_PARAM;
    }

    uint8_t fillValue = (color == SSD1306_COLOR_WHITE) ? 0xFF : 0x00;
    memset(SSD1306_Buffer, fillValue, SSD1306_BUFFER_SIZE);

    log_debug("SSD1306: Display buffer filled with %s", color == SSD1306_COLOR_WHITE ? "white" : "black");
    return SSD1306_OK;
}

/**
 * @brief   Draw pixel at specified coordinates
 * @param   hssd Pointer to SSD1306 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   color Pixel color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_DrawPixel(SSD1306_Handle_t *hssd,
                                       uint16_t x, uint16_t y,
                                       SSD1306_Color_t color)
{
    if (hssd == NULL || x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) {
        log_error("SSD1306: Invalid parameters for DrawPixel - x:%d, y:%d, max:%dx%d", x, y, SSD1306_WIDTH, SSD1306_HEIGHT);
        return SSD1306_INVALID_PARAM;
    }

    if (color == SSD1306_COLOR_WHITE) {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= (1 << (y % 8));
    } else {
        SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
    }

    return SSD1306_OK;
}

/**
 * @brief   Write character
 * @param   hssd Pointer to SSD1306 handle
 * @param   ch Character to write
 * @param   color Character color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_WriteChar(SSD1306_Handle_t *hssd, char ch, SSD1306_Color_t color)
{
    if (hssd == NULL || !hssd->initialized) {
        return SSD1306_NOT_INITIALIZED;
    }

    /* Only SSD1306_FONT_6x8 exists, so the dimensions are fixed. */
    const uint8_t fontWidth = SSD1306_FONT_WIDTH;
    const uint8_t fontHeight = SSD1306_FONT_HEIGHT;

    if (hssd->currentX + fontWidth > SSD1306_WIDTH) {
        hssd->currentX = 0;
        hssd->currentY += fontHeight;
    }

    if (hssd->currentY + fontHeight > SSD1306_HEIGHT) {
        return SSD1306_INVALID_PARAM;
    }

    if (ch < SSD1306_FONT_FIRST_CHAR || ch > SSD1306_FONT_LAST_CHAR) {
        return SSD1306_INVALID_PARAM;
    }

    const uint8_t *glyph = font6x8[(uint8_t)ch - SSD1306_FONT_FIRST_CHAR];

    for (uint8_t i = 0; i < fontWidth; i++) {
        uint8_t line = glyph[i];
        for (uint8_t j = 0; j < fontHeight; j++) {
            if (line & (1u << j)) {
                SSD1306_DrawPixel(hssd, hssd->currentX + i, hssd->currentY + j, color);
            }
        }
    }

    hssd->currentX += fontWidth;

    return SSD1306_OK;
}

/**
 * @brief   Write string
 * @param   hssd Pointer to SSD1306 handle
 * @param   str String to write
 * @param   color String color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_WriteString(SSD1306_Handle_t *hssd, const char *str, SSD1306_Color_t color)
{
    if (hssd == NULL || str == NULL) {
        return SSD1306_INVALID_PARAM;
    }

    while (*str) {
        if (SSD1306_WriteChar(hssd, *str, color) != SSD1306_OK) {
            return SSD1306_ERROR;
        }
        str++;
    }

    return SSD1306_OK;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Write command to SSD1306
 * @param   hssd Pointer to SSD1306 handle
 * @param   command Command byte
 * @retval  SSD1306_StatusTypeDef Operation status
 */
static SSD1306_StatusTypeDef SSD1306_WriteCommand(SSD1306_Handle_t *hssd, uint8_t command)
{
    uint8_t data[2] = {0x00, command}; // 0x00 = command mode

    if (I2C_Master_Transmit(hssd->config.address, data, 2, I2C_TIMEOUT_DEFAULT) != I2C_OK) {
        log_error("SSD1306: Failed to write command 0x%02X", command);
        return SSD1306_ERROR;
    }

    return SSD1306_OK;
}

/**
 * @brief   Write data to SSD1306
 * @param   hssd Pointer to SSD1306 handle
 * @param   data Data buffer
 * @param   size Data size
 * @retval  SSD1306_StatusTypeDef Operation status
 */
static SSD1306_StatusTypeDef SSD1306_WriteData(SSD1306_Handle_t *hssd, uint8_t *data, uint16_t size)
{
    if (size > SSD1306_BUFFER_SIZE) {
        log_error("SSD1306: Data size %d exceeds buffer size %d", size, SSD1306_BUFFER_SIZE);
        return SSD1306_INVALID_PARAM;
    }

    ssd1306_tx_buffer[0] = 0x40; // Data mode
    memcpy(&ssd1306_tx_buffer[1], data, size);

    if (I2C_Master_Transmit(hssd->config.address, ssd1306_tx_buffer, size + 1, I2C_TIMEOUT_DEFAULT) != I2C_OK) {
        log_error("SSD1306: Failed to write %d bytes of data", size);
        return SSD1306_ERROR;
    }

    return SSD1306_OK;
}
