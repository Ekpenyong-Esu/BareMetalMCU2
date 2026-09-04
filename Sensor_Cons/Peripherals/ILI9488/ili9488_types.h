/**
 ******************************************************************************
 * @file    ili9488_types.h
 * @brief   Shared vocabulary for the ILI9488 TFT driver
 ******************************************************************************
 */

#ifndef ILI9488_TYPES_H
#define ILI9488_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include "spi_types.h"
#include <stdint.h>
#include <stdbool.h>

/** @defgroup ILI9488_Display_Specifications Display Specifications
 * @{
 */
#define ILI9488_WIDTH 320
#define ILI9488_HEIGHT 480
#define ILI9488_PIXEL_COUNT (ILI9488_WIDTH * ILI9488_HEIGHT)

/* Geometry of the built-in font table */
#define ILI9488_FONT_WIDTH 6U
#define ILI9488_FONT_HEIGHT 8U
#define ILI9488_FONT_FIRST_CHAR 32U
#define ILI9488_FONT_CHAR_COUNT 96U

/* Color definitions (RGB565) */
#define ILI9488_COLOR_BLACK 0x0000
#define ILI9488_COLOR_WHITE 0xFFFF
#define ILI9488_COLOR_RED 0xF800
#define ILI9488_COLOR_GREEN 0x07E0
#define ILI9488_COLOR_BLUE 0x001F
#define ILI9488_COLOR_YELLOW 0xFFE0
#define ILI9488_COLOR_MAGENTA 0xF81F
#define ILI9488_COLOR_CYAN 0x07FF
/** @} */

/**
 * @brief ILI9488 Status enumeration
 */
typedef enum {
    ILI9488_OK = 0,         /**< Operation completed successfully */
    ILI9488_ERROR,          /**< General error occurred */
    ILI9488_BUSY,           /**< Display is busy */
    ILI9488_TIMEOUT,        /**< Operation timed out */
    ILI9488_INVALID_PARAM,  /**< Invalid parameter provided */
    ILI9488_NOT_INITIALIZED /**< Driver not initialized */
} ILI9488_StatusTypeDef;

/**
 * @brief ILI9488 Orientation enumeration
 */
typedef enum {
    ILI9488_ORIENTATION_PORTRAIT = 0, /**< Portrait orientation */
    ILI9488_ORIENTATION_LANDSCAPE,    /**< Landscape orientation */
    ILI9488_ORIENTATION_PORTRAIT_REV, /**< Portrait reversed */
    ILI9488_ORIENTATION_LANDSCAPE_REV /**< Landscape reversed */
} ILI9488_Orientation_t;

/**
 * @brief ILI9488 Configuration structure
 * @note  The bus is opened by the application; the driver only owns the
 *        panel's control lines and its slot on that bus.
 */
typedef struct {
    SPI_Bus_t *bus;                    /**< Bus the panel is wired to, already open */
    GPIO_TypeDef *cs_port;             /**< Chip select port */
    uint16_t cs_pin;                   /**< Chip select pin */
    GPIO_TypeDef *dc_port;             /**< Data/command port */
    uint16_t dc_pin;                   /**< Data/command pin */
    GPIO_TypeDef *rst_port;            /**< Reset port */
    uint16_t rst_pin;                  /**< Reset pin */
    ILI9488_Orientation_t orientation; /**< Display orientation */
} ILI9488_Config_t;

/**
 * @brief ILI9488 Handle structure
 */
typedef struct {
    SPI_Device_t device;     /**< This panel's slot on the bus */
    ILI9488_Config_t config; /**< Display configuration */
    uint16_t currentX;       /**< Current X position */
    uint16_t currentY;       /**< Current Y position */
    uint16_t width;          /**< Display width */
    uint16_t height;         /**< Display height */
    bool initialized;        /**< Initialization status */
} ILI9488_Handle_t;

/** Reject a handle that is NULL or whose panel was never brought up */
#define ILI9488_CHECK_HANDLE(hili)                                                                 \
    do {                                                                                           \
        if ((hili) == NULL) {                                                                      \
            return ILI9488_INVALID_PARAM;                                                          \
        }                                                                                          \
        if (!(hili)->initialized) {                                                                \
            return ILI9488_NOT_INITIALIZED;                                                        \
        }                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* ILI9488_TYPES_H */
