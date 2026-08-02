/**
  ******************************************************************************
  * @file    ssd1306.h
  * @brief   SSD1306 OLED Display Driver for STM32F429I-DISC1
  * @details This file contains function prototypes and definitions for
  *          SSD1306 128x64 OLED display control using I2C interface.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

#ifndef __SSD1306_H__
#define __SSD1306_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "i2c.h"
#include <stdint.h>
#include <stdbool.h>
#include "log.h"

/* Exported constants --------------------------------------------------------*/

/** @defgroup SSD1306_Display_Specifications Display Specifications
 * @{
 */
#define SSD1306_WIDTH                    128
#define SSD1306_HEIGHT                   64
#define SSD1306_BUFFER_SIZE              (SSD1306_WIDTH * SSD1306_HEIGHT / 8)
#define SSD1306_FONT_WIDTH               6
#define SSD1306_FONT_HEIGHT              8

#define SSD1306_I2C_ADDR                 0x3C    // Default I2C address
#define SSD1306_I2C_ADDR_ALT             0x3D    // Alternative I2C address

/** @} */

/* Exported types ------------------------------------------------------------*/

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
    SSD1306_COLOR_BLACK = 0,        /**< Black color */
    SSD1306_COLOR_WHITE = 1         /**< White color */
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
    uint8_t address;                /**< I2C address */
} SSD1306_Config_t;

/**
 * @brief SSD1306 Handle structure
 */
typedef struct {
    SSD1306_Config_t config;        /**< Display configuration */
    uint16_t currentX;              /**< Current X position */
    uint16_t currentY;              /**< Current Y position */
    bool initialized;               /**< Initialization status */
} SSD1306_Handle_t;

/* Global display buffer - shared across all instances */
extern uint8_t SSD1306_Buffer[SSD1306_BUFFER_SIZE];

/* Exported functions -------------------------------------------------------*/

/** @defgroup SSD1306_Init Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize SSD1306 OLED display
 * @details Configures I2C and initializes the display
 * @param   hssd Pointer to SSD1306 handle
 * @param   address I2C address (0x3C or 0x3D)
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Init(SSD1306_Handle_t *hssd, uint8_t address);

/** @} */

/** @defgroup SSD1306_Display Display Control
 * @{
 */

/**
 * @brief   Update display with buffer content
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_UpdateScreen(SSD1306_Handle_t *hssd);

/**
 * @brief   Clear display buffer
 * @param   hssd Pointer to SSD1306 handle
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Clear(SSD1306_Handle_t *hssd);

/**
 * @brief   Fill display buffer with color
 * @param   hssd Pointer to SSD1306 handle
 * @param   color Fill color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_Fill(SSD1306_Handle_t *hssd, SSD1306_Color_t color);

/** @} */

/** @defgroup SSD1306_Graphics Graphics Functions
 * @{
 */

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
                                       SSD1306_Color_t color);

/** @} */

/** @defgroup SSD1306_Text Text Functions
 * @{ */

/**
 * @brief   Write character
 * @param   hssd Pointer to SSD1306 handle
 * @param   ch Character to write
 * @param   color Character color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_WriteChar(SSD1306_Handle_t *hssd, char ch, SSD1306_Color_t color);

/**
 * @brief   Write string
 * @param   hssd Pointer to SSD1306 handle
 * @param   str String to write
 * @param   color String color
 * @retval  SSD1306_StatusTypeDef Operation status
 */
SSD1306_StatusTypeDef SSD1306_WriteString(SSD1306_Handle_t *hssd, const char *str, SSD1306_Color_t color);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __SSD1306_H__ */
