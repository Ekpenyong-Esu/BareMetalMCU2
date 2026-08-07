/**
  ******************************************************************************
  * @file    ssd1306_types.h
  * @brief   SSD1306 OLED driver data definitions
  ******************************************************************************
  */

#ifndef SSD1306_TYPES_H
#define SSD1306_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/** @defgroup SSD1306_Display_Specifications Display Specifications
 * @{
 */
#define SSD1306_WIDTH                    128
#define SSD1306_HEIGHT                   64
#define SSD1306_PAGE_COUNT               (SSD1306_HEIGHT / 8)
#define SSD1306_BUFFER_SIZE              (SSD1306_WIDTH * SSD1306_PAGE_COUNT)
#define SSD1306_FONT_WIDTH               6
#define SSD1306_FONT_HEIGHT              8

/* 7-bit addresses; the driver shifts them for the HAL. */
#define SSD1306_I2C_ADDR                 0x3C
#define SSD1306_I2C_ADDR_ALT             0x3D
/** @} */

/**
 * @brief SSD1306 Status enumeration
 */
typedef enum {
    SSD1306_OK = 0,                 /**< Operation completed successfully */
    SSD1306_ERROR,                  /**< General error occurred */
    SSD1306_BUSY,                   /**< Display is busy */
    SSD1306_TIMEOUT,                /**< Operation timed out */
    SSD1306_INVALID_PARAM,          /**< Invalid parameter provided */
    SSD1306_NOT_INITIALIZED         /**< Driver not initialized */
} SSD1306_StatusTypeDef;

/**
 * @brief SSD1306 Color enumeration
 */
typedef enum {
    SSD1306_COLOR_BLACK = 0,        /**< Pixel off */
    SSD1306_COLOR_WHITE = 1         /**< Pixel on */
} SSD1306_Color_t;

/**
 * @brief SSD1306 Font size enumeration
 */
typedef enum {
    SSD1306_FONT_6x8 = 0            /**< 6x8 font (only supported size) */
} SSD1306_FontSize_t;

/**
 * @brief SSD1306 Configuration structure
 */
typedef struct {
    uint8_t address;                /**< 7-bit I2C address */
} SSD1306_Config_t;

/**
 * @brief SSD1306 Handle structure
 */
typedef struct {
    SSD1306_Config_t config;        /**< Display configuration */
    uint16_t currentX;              /**< Text cursor column in pixels */
    uint16_t currentY;              /**< Text cursor row in pixels */
    bool initialized;               /**< Initialization status */
} SSD1306_Handle_t;

/**
 * @brief Reject a NULL or uninitialised handle at every entry point.
 */
#define SSD1306_CHECK_HANDLE(hssd)                     \
    do {                                               \
        if ((hssd) == NULL) {                          \
            return SSD1306_INVALID_PARAM;              \
        }                                              \
        if (!(hssd)->initialized) {                    \
            return SSD1306_NOT_INITIALIZED;            \
        }                                              \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SSD1306_TYPES_H */
