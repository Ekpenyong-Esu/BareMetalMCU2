/**
  ******************************************************************************
  * @file    xpt2046.h
  * @brief   XPT2046 Resistive Touchscreen Controller Driver for STM32F429I-DISC1
  * @details This file contains function prototypes and definitions for
  *          XPT2046 resistive touchscreen controller using SPI interface.
  * @version 1.0
  * @date    2025-01-19
  ******************************************************************************
  */

#ifndef __XPT2046_H__
#define __XPT2046_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "spi.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/** @defgroup XPT2046_Display_Specifications Specifications
 * @{
 */
#define XPT2046_MAX_PRESSURE           4095    /**< Maximum pressure value */

/* Typical raw span of a resistive panel: the touch layer never reaches the ADC
   rails, so mapping from 0..4095 would make the screen edges unreachable. */
#define XPT2046_RAW_X_MIN_DEFAULT      200     /**< Default raw X at the left edge */
#define XPT2046_RAW_X_MAX_DEFAULT      3900    /**< Default raw X at the right edge */
#define XPT2046_RAW_Y_MIN_DEFAULT      200     /**< Default raw Y at the top edge */
#define XPT2046_RAW_Y_MAX_DEFAULT      3900    /**< Default raw Y at the bottom edge */

/* Touch states */
#define XPT2046_STATE_RELEASED         0       /**< Touch released */
#define XPT2046_STATE_PRESSED          1       /**< Touch pressed */
#define XPT2046_STATE_HELD             2       /**< Touch held */

/** @} */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief XPT2046 Status enumeration
 */
typedef enum {
    XPT2046_OK = 0,                 /**< Operation completed successfully */
    XPT2046_ERROR,                  /**< General error occurred */
    XPT2046_BUSY,                   /**< Controller is busy */
    XPT2046_TIMEOUT,                /**< Operation timed out */
    XPT2046_INVALID_PARAM,          /**< Invalid parameter provided */
    XPT2046_NOT_INITIALIZED,        /**< Driver not initialized */
    XPT2046_NO_TOUCH                /**< No touch detected */
} XPT2046_StatusTypeDef;

/**
 * @brief XPT2046 Touch point structure
 */
typedef struct {
    uint16_t x;                     /**< X coordinate */
    uint16_t y;                     /**< Y coordinate */
    uint16_t pressure;              /**< Touch pressure */
    uint8_t state;                  /**< Touch state */
} XPT2046_TouchPoint_t;

/**
 * @brief XPT2046 Configuration structure
 */
typedef struct {
    GPIO_TypeDef *cs_port;          /**< Chip select port */
    uint16_t cs_pin;                /**< Chip select pin */
    GPIO_TypeDef *irq_port;         /**< Interrupt port */
    uint16_t irq_pin;               /**< Interrupt pin */
    uint16_t width;                 /**< Display width for coordinate mapping */
    uint16_t height;                /**< Display height for coordinate mapping */
    uint16_t raw_x_min;             /**< Raw X reported at the left edge */
    uint16_t raw_x_max;             /**< Raw X reported at the right edge */
    uint16_t raw_y_min;             /**< Raw Y reported at the top edge */
    uint16_t raw_y_max;             /**< Raw Y reported at the bottom edge */
    bool flip_x;                    /**< Flip X coordinates */
    bool flip_y;                    /**< Flip Y coordinates */
} XPT2046_Config_t;

/**
 * @brief XPT2046 Handle structure
 */
typedef struct {
    XPT2046_Config_t config;        /**< Configuration */
    XPT2046_TouchPoint_t touch;     /**< Current touch point */
    bool initialized;               /**< Initialization status */
} XPT2046_Handle_t;

/* Exported functions -------------------------------------------------------*/

/** @defgroup XPT2046_Init Initialization and Configuration
 * @{
 */

/**
 * @brief   Initialize XPT2046 touchscreen controller
 * @details Configures SPI and initializes the touchscreen
 * @param   hxpt Pointer to XPT2046 handle
 * @param   cs_port Chip select port
 * @param   cs_pin Chip select pin
 * @param   irq_port Interrupt port
 * @param   irq_pin Interrupt pin
 * @param   width Display width
 * @param   height Display height
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Init(XPT2046_Handle_t *hxpt,
                                  GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                  GPIO_TypeDef *irq_port, uint16_t irq_pin,
                                  uint16_t width, uint16_t height);

/**
 * @brief   Replace the default raw span with values measured on the panel
 * @details Prompting the user for the corner touches belongs to the application.
 * @param   hxpt Pointer to XPT2046 handle
 * @param   raw_x_min Raw X read at the left edge
 * @param   raw_x_max Raw X read at the right edge
 * @param   raw_y_min Raw Y read at the top edge
 * @param   raw_y_max Raw Y read at the bottom edge
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_SetCalibration(XPT2046_Handle_t *hxpt,
                                            uint16_t raw_x_min, uint16_t raw_x_max,
                                            uint16_t raw_y_min, uint16_t raw_y_max);


/** @} */

/** @defgroup XPT2046_Touch Touch Detection and Reading
 * @{
 */

/**
 * @brief   Check if touchscreen is touched
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  bool True if touched, false otherwise
 */
bool XPT2046_IsTouched(XPT2046_Handle_t *hxpt);

/**
 * @brief   Read touch coordinates and pressure
 * @param   hxpt Pointer to XPT2046 handle
 * @param   touch Pointer to touch point structure
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_ReadTouch(XPT2046_Handle_t *hxpt, XPT2046_TouchPoint_t *touch);

/**
 * @brief   Update touch state
 * @param   hxpt Pointer to XPT2046 handle
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Update(XPT2046_Handle_t *hxpt);

/** @} */

#ifdef __cplusplus
}
#endif

#endif /* __XPT2046_H__ */
