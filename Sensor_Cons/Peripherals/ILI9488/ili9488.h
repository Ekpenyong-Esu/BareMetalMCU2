/**
  ******************************************************************************
  * @file    ili9488.h
  * @brief   ILI9488 TFT LCD Display Driver for STM32F429I-DISC1
  * @details This file contains function prototypes and definitions for
  *          ILI9488 4-inch TFT LCD display control using SPI interface.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

#ifndef __ILI9488_H__
#define __ILI9488_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup ILI9488_Display_Specifications Display Specifications
 * @{
 */
#define ILI9488_WIDTH                    320
#define ILI9488_HEIGHT                   480
#define ILI9488_PIXEL_COUNT              (ILI9488_WIDTH * ILI9488_HEIGHT)

/* Geometry of the built-in font table */
#define ILI9488_FONT_WIDTH               6U
#define ILI9488_FONT_HEIGHT              8U
#define ILI9488_FONT_FIRST_CHAR          32U
#define ILI9488_FONT_CHAR_COUNT          96U

/* Color definitions */
#define ILI9488_COLOR_BLACK              0x0000
#define ILI9488_COLOR_WHITE              0xFFFF
#define ILI9488_COLOR_RED                0xF800
#define ILI9488_COLOR_GREEN              0x07E0
#define ILI9488_COLOR_BLUE               0x001F
#define ILI9488_COLOR_YELLOW             0xFFE0
#define ILI9488_COLOR_MAGENTA            0xF81F
#define ILI9488_COLOR_CYAN               0x07FF

/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief ILI9488 Status enumeration
 */
typedef enum {
    ILI9488_OK = 0,                 /**< Operation completed successfully */
    ILI9488_ERROR,                  /**< General error occurred */
    ILI9488_BUSY,                   /**< Display is busy */
    ILI9488_TIMEOUT,                /**< Operation timed out */
    ILI9488_INVALID_PARAM,          /**< Invalid parameter provided */
    ILI9488_NOT_INITIALIZED         /**< Driver not initialized */
} ILI9488_StatusTypeDef;

/**
 * @brief ILI9488 Orientation enumeration
 */
typedef enum {
    ILI9488_ORIENTATION_PORTRAIT = 0,     /**< Portrait orientation */
    ILI9488_ORIENTATION_LANDSCAPE,        /**< Landscape orientation */
    ILI9488_ORIENTATION_PORTRAIT_REV,     /**< Portrait reversed */
    ILI9488_ORIENTATION_LANDSCAPE_REV     /**< Landscape reversed */
} ILI9488_Orientation_t;

/**
 * @brief ILI9488 Configuration structure
 */
typedef struct {
    GPIO_TypeDef *cs_port;          /**< Chip select port */
    uint16_t cs_pin;                /**< Chip select pin */
    GPIO_TypeDef *dc_port;          /**< Data/command port */
    uint16_t dc_pin;                /**< Data/command pin */
    GPIO_TypeDef *rst_port;         /**< Reset port */
    uint16_t rst_pin;               /**< Reset pin */
    ILI9488_Orientation_t orientation; /**< Display orientation */
} ILI9488_Config_t;

/**
 * @brief ILI9488 Handle structure
 */
typedef struct {
    ILI9488_Config_t config;        /**< Display configuration */
    uint16_t currentX;              /**< Current X position */
    uint16_t currentY;              /**< Current Y position */
    uint16_t width;                 /**< Display width */
    uint16_t height;                /**< Display height */
    bool initialized;               /**< Initialization status */
} ILI9488_Handle_t;

/* Exported functions -------------------------------------------------------*/

/** @defgroup ILI9488_Init Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize ILI9488 TFT display
 * @details Configures SPI and initializes the display
 * @param   hili Pointer to ILI9488 handle
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   dc_port Data/command port
 * @param   dc_pin Data/command pin
 * @param   rst_port Reset port
 * @param   rst_pin Reset pin
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_Init(ILI9488_Handle_t *hili,
                                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                  GPIO_TypeDef *dc_port, uint16_t dc_pin,
                                  GPIO_TypeDef *rst_port, uint16_t rst_pin);

/**
 * @brief   Set display orientation
 * @param   hili Pointer to ILI9488 handle
 * @param   orientation Display orientation
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_SetOrientation(ILI9488_Handle_t *hili, ILI9488_Orientation_t orientation);

/** @} */

/** @defgroup ILI9488_Display Display Control
 * @{
 */

/**
 * @brief   Clear display
 * @param   hili Pointer to ILI9488 handle
 * @param   color Fill color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_Clear(ILI9488_Handle_t *hili, uint16_t color);

/** @} */

/** @defgroup ILI9488_Graphics Graphics Functions
 * @{
 */

/**
 * @brief   Draw pixel at specified coordinates
 * @param   hili Pointer to ILI9488 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @param   color Pixel color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_DrawPixel(ILI9488_Handle_t *hili,
                                       uint16_t x, uint16_t y,
                                       uint16_t color);

/** @} */

/** @defgroup ILI9488_Text Text Functions
 * @{
 */

/**
 * @brief   Set cursor position
 * @param   hili Pointer to ILI9488 handle
 * @param   x X coordinate
 * @param   y Y coordinate
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_SetCursor(ILI9488_Handle_t *hili, uint16_t x, uint16_t y);

/**
 * @brief   Write character
 * @param   hili Pointer to ILI9488 handle
 * @param   ch Character to write
 * @param   color Character color
 * @param   bgcolor Background color
 * @retval  ILI9488_StatusTypeDef Operation status
 */
ILI9488_StatusTypeDef ILI9488_WriteChar(ILI9488_Handle_t *hili, char ch, uint16_t color, uint16_t bgcolor);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __ILI9488_H__ */
