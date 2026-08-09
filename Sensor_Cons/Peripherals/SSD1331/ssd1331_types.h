/**
  ******************************************************************************
  * @file    ssd1331_types.h
  * @brief   SSD1331 colour OLED driver data definitions
  ******************************************************************************
  */

#ifndef SSD1331_TYPES_H
#define SSD1331_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/** @defgroup SSD1331_Display_Specifications Display Specifications
 * @{
 */
#define SSD1331_WIDTH                    96
#define SSD1331_HEIGHT                   64
#define SSD1331_BYTES_PER_PIXEL          2
#define SSD1331_BUFFER_SIZE              (SSD1331_WIDTH * SSD1331_HEIGHT * SSD1331_BYTES_PER_PIXEL)
#define SSD1331_FONT_WIDTH               6
#define SSD1331_FONT_HEIGHT              8
/** @} */

/**
 * @brief SSD1331 Status enumeration
 */
typedef enum {
    SSD1331_OK = 0,                 /**< Operation completed successfully */
    SSD1331_ERROR,                  /**< General error occurred */
    SSD1331_BUSY,                   /**< Display is busy */
    SSD1331_TIMEOUT,                /**< Operation timed out */
    SSD1331_INVALID_PARAM,          /**< Invalid parameter provided */
    SSD1331_NOT_INITIALIZED         /**< Driver not initialized */
} SSD1331_StatusTypeDef;

/**
 * @brief   RGB565 colour, the format the panel is configured for.
 */
typedef uint16_t SSD1331_Color_t;

/**
 * @brief   Build an RGB565 colour from 8-bit components.
 * @note    The low bits of each component are discarded: the panel carries
 *          5 bits of red, 6 of green and 5 of blue.
 */
#define SSD1331_RGB(r, g, b)                                    \
    ((SSD1331_Color_t)((((uint16_t)(r) & 0xF8U) << 8) |         \
                       (((uint16_t)(g) & 0xFCU) << 3) |         \
                        ((uint16_t)(b) >> 3)))

/** @defgroup SSD1331_Colors Ready-made colours
 * @{
 */
#define SSD1331_COLOR_BLACK              ((SSD1331_Color_t)0x0000)
#define SSD1331_COLOR_WHITE              ((SSD1331_Color_t)0xFFFF)
#define SSD1331_COLOR_RED                ((SSD1331_Color_t)0xF800)
#define SSD1331_COLOR_GREEN              ((SSD1331_Color_t)0x07E0)
#define SSD1331_COLOR_BLUE               ((SSD1331_Color_t)0x001F)
#define SSD1331_COLOR_YELLOW             ((SSD1331_Color_t)0xFFE0)
#define SSD1331_COLOR_CYAN               ((SSD1331_Color_t)0x07FF)
#define SSD1331_COLOR_MAGENTA            ((SSD1331_Color_t)0xF81F)
/** @} */

/**
 * @brief SSD1331 Configuration structure
 * @details The panel shares the SPI bus but needs three private control lines.
 *          Reset may be left out by setting @p rstPort to NULL, in which case
 *          the module's RES pin must be tied high on the board.
 */
typedef struct {
    GPIO_TypeDef *csPort;           /**< Chip select port */
    uint16_t      csPin;            /**< Chip select pin */
    GPIO_TypeDef *dcPort;           /**< Data/command select port */
    uint16_t      dcPin;            /**< Data/command select pin */
    GPIO_TypeDef *rstPort;          /**< Reset port, NULL if RES is tied high */
    uint16_t      rstPin;           /**< Reset pin */
} SSD1331_Config_t;

/**
 * @brief SSD1331 Handle structure
 */
typedef struct {
    SSD1331_Config_t config;        /**< Display configuration */
    uint16_t currentX;              /**< Text cursor column in pixels */
    uint16_t currentY;              /**< Text cursor row in pixels */
    bool initialized;               /**< Initialization status */
} SSD1331_Handle_t;

/**
 * @brief Reject a NULL or uninitialised handle at every entry point.
 */
#define SSD1331_CHECK_HANDLE(hssd)                     \
    do {                                               \
        if ((hssd) == NULL) {                          \
            return SSD1331_INVALID_PARAM;              \
        }                                              \
        if (!(hssd)->initialized) {                    \
            return SSD1331_NOT_INITIALIZED;            \
        }                                              \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SSD1331_TYPES_H */
