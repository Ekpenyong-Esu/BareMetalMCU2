/**
 * @file ltdc_types.h
 * @brief Shared vocabulary for the LTDC (RGB interface) driver
 * @details Panel geometry, pixel formats, colours, error codes and the driver
 *          record. No behaviour lives here, so every LTDC module can include it
 *          without depending on any other module.
 */

#ifndef LTDC_TYPES_H
#define LTDC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* Display specifications for STM32F429I-DISC1 ------------------------------*/
#define LTDC_DISPLAY_WIDTH          240     /*!< Display width in pixels */
#define LTDC_DISPLAY_HEIGHT         320     /*!< Display height in pixels */
#define LTDC_MAX_LAYERS             2       /*!< Maximum number of display layers */

/* ILI9341 RGB panel porch timings (true widths, not accumulated register values) */
#define LTDC_HSYNC_WIDTH            10       /*!< Horizontal sync width */
#define LTDC_VSYNC_HEIGHT           2        /*!< Vertical sync height */
#define LTDC_HBP_WIDTH              20       /*!< Horizontal back porch */
#define LTDC_VBP_HEIGHT             2        /*!< Vertical back porch */
#define LTDC_HFP_WIDTH              10       /*!< Horizontal front porch */
#define LTDC_VFP_HEIGHT             4        /*!< Vertical front porch */

/* Memory allocation constants -----------------------------------------------*/
#define LTDC_BYTES_PER_PIXEL_RGB565   2     /*!< Bytes per pixel for RGB565 format */
#define LTDC_BYTES_PER_PIXEL_RGB888   3     /*!< Bytes per pixel for RGB888 format */
#define LTDC_BYTES_PER_PIXEL_ARGB8888 4     /*!< Bytes per pixel for ARGB8888 format */

/* Framebuffer size calculations */
#define LTDC_FB_SIZE_RGB565         (LTDC_DISPLAY_WIDTH * LTDC_DISPLAY_HEIGHT * LTDC_BYTES_PER_PIXEL_RGB565)
#define LTDC_FB_SIZE_RGB888         (LTDC_DISPLAY_WIDTH * LTDC_DISPLAY_HEIGHT * LTDC_BYTES_PER_PIXEL_RGB888)
#define LTDC_FB_SIZE_ARGB8888       (LTDC_DISPLAY_WIDTH * LTDC_DISPLAY_HEIGHT * LTDC_BYTES_PER_PIXEL_ARGB8888)

/* Default colors in RGB565 format ------------------------------------------*/
#define LTDC_COLOR_BLACK            0x0000  /*!< Black color */
#define LTDC_COLOR_WHITE            0xFFFF  /*!< White color */
#define LTDC_COLOR_RED              0xF800  /*!< Red color */
#define LTDC_COLOR_GREEN            0x07E0  /*!< Green color */
#define LTDC_COLOR_BLUE             0x001F  /*!< Blue color */
#define LTDC_COLOR_YELLOW           0xFFE0  /*!< Yellow color */
#define LTDC_COLOR_CYAN             0x07FF  /*!< Cyan color */
#define LTDC_COLOR_MAGENTA          0xF81F  /*!< Magenta color */
#define LTDC_COLOR_GRAY             0x7BEF  /*!< Gray color */
#define LTDC_COLOR_DARKGRAY         0x39E7  /*!< Dark gray color */
#define LTDC_COLOR_LIGHTGRAY        0xBDF7  /*!< Light gray color */

/* Error codes ---------------------------------------------------------------*/
#define LTDC_ERROR_NONE                  0x00    /*!< No error */
#define LTDC_ERROR_INVALID_PARAM         0x01    /*!< Invalid parameter */
#define LTDC_ERROR_INIT_FAILED           0x02    /*!< Initialization failed */
#define LTDC_ERROR_LAYER_CONFIG          0x03    /*!< Layer configuration failed */
#define LTDC_ERROR_MEMORY_ALLOC          0x04    /*!< Memory allocation failed */
#define LTDC_ERROR_INVALID_LAYER         0x05    /*!< Invalid layer number */
#define LTDC_ERROR_FRAMEBUFFER           0x06    /*!< Framebuffer error */
#define LTDC_ERROR_UNSUPPORTED_FORMAT    0x07    /*!< Unsupported pixel format (avoid RGB888) */

/* Data Types ----------------------------------------------------------------*/

/**
 * @brief LTDC pixel format enumeration
 */
typedef enum {
    LTDC_PIXEL_FORMAT_ARGB8888_ENUM = 0,    /*!< 32-bit ARGB8888 format */
    LTDC_PIXEL_FORMAT_RGB888_ENUM,          /*!< 24-bit RGB888 format */
    LTDC_PIXEL_FORMAT_RGB565_ENUM,          /*!< 16-bit RGB565 format */
    LTDC_PIXEL_FORMAT_ARGB1555_ENUM,        /*!< 16-bit ARGB1555 format */
    LTDC_PIXEL_FORMAT_ARGB4444_ENUM,        /*!< 16-bit ARGB4444 format */
    LTDC_PIXEL_FORMAT_L8_ENUM,              /*!< 8-bit luminance format */
    LTDC_PIXEL_FORMAT_AL44_ENUM,            /*!< 8-bit alpha-luminance format */
    LTDC_PIXEL_FORMAT_AL88_ENUM             /*!< 16-bit alpha-luminance format */
} LTDC_PixelFormat_t;

/**
 * @brief LTDC blending mode enumeration
 */
typedef enum {
    LTDC_BLEND_CONSTANT_ALPHA = 0,          /*!< Constant alpha blending */
    LTDC_BLEND_PIXEL_ALPHA,                 /*!< Pixel alpha blending */
    LTDC_BLEND_NO_BLENDING                  /*!< No blending (opaque) */
} LTDC_BlendMode_t;

/**
 * @brief LTDC layer configuration structure
 */
typedef struct {
    uint32_t framebufferAddress;            /*!< Framebuffer start address */
    uint16_t windowX0;                      /*!< Window left position */
    uint16_t windowY0;                      /*!< Window top position */
    uint16_t windowX1;                      /*!< Window right position (inclusive) */
    uint16_t windowY1;                      /*!< Window bottom position (inclusive) */
    uint16_t imageWidth;                    /*!< Image width in pixels */
    uint16_t imageHeight;                   /*!< Image height in pixels */
    LTDC_PixelFormat_t pixelFormat;         /*!< Pixel format */
    uint8_t alpha;                          /*!< Layer alpha value (0-255) */
    uint8_t alpha0;                         /*!< Transparent pixel alpha */
    LTDC_BlendMode_t blendMode;             /*!< Blending mode */
    uint32_t backgroundColor;               /*!< Layer background color */
    bool enabled;                           /*!< Layer enable status */
} LTDC_LayerConfig_t;

/**
 * @brief LTDC display configuration structure
 */
typedef struct {
    uint16_t width;                         /*!< Display width */
    uint16_t height;                        /*!< Display height */
    uint32_t backgroundColor;               /*!< Display background color */
    bool hsyncActiveLow;                    /*!< Horizontal sync polarity */
    bool vsyncActiveLow;                    /*!< Vertical sync polarity */
    bool dataEnableActiveLow;               /*!< Data enable polarity */
    bool pixelClockInverted;                /*!< Pixel clock polarity */
} LTDC_DisplayConfig_t;

/**
 * @brief LTDC driver handle structure
 */
typedef struct {
    LTDC_HandleTypeDef *hltdc;                  /*!< HAL LTDC handle */
    LTDC_DisplayConfig_t displayConfig;         /*!< Display configuration */
    LTDC_LayerConfig_t layers[LTDC_MAX_LAYERS]; /*!< Layer configurations */
    uint8_t activeLayer;                        /*!< Currently active layer */
    bool initialized;                           /*!< Initialization status */
    uint32_t errorCode;                         /*!< Last error code */
    volatile uint8_t reloadFlag;                /*!< Set when a reload event occurs (VSYNC) */
} LTDC_Driver_t;

/**
 * @brief Rectangle structure
 */
typedef struct {
    uint16_t x;                             /*!< X coordinate */
    uint16_t y;                             /*!< Y coordinate */
    uint16_t width;                         /*!< Rectangle width */
    uint16_t height;                        /*!< Rectangle height */
} LTDC_Rect_t;

/**
 * @brief Point structure
 */
typedef struct {
    uint16_t x;                             /*!< X coordinate */
    uint16_t y;                             /*!< Y coordinate */
} LTDC_Point_t;

#ifdef __cplusplus
}
#endif

#endif /* LTDC_TYPES_H */
