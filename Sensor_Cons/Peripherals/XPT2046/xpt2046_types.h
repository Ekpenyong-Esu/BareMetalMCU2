/**
 * @file xpt2046_types.h
 * @brief Data types and constants for the XPT2046 resistive touch controller
 */

#ifndef XPT2046_TYPES_H
#define XPT2046_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/** @brief Full-scale value of the 12-bit converter */
#define XPT2046_ADC_MAX                4095U

/** @brief Upper bound of the reported pressure figure */
#define XPT2046_MAX_PRESSURE           4095U

/* Typical raw span of a resistive panel: the touch layer never reaches the ADC
   rails, so mapping from 0..4095 would make the screen edges unreachable. */
#define XPT2046_RAW_X_MIN_DEFAULT      200U    /**< Default raw X at the left edge */
#define XPT2046_RAW_X_MAX_DEFAULT      3900U   /**< Default raw X at the right edge */
#define XPT2046_RAW_Y_MIN_DEFAULT      200U    /**< Default raw Y at the top edge */
#define XPT2046_RAW_Y_MAX_DEFAULT      3900U   /**< Default raw Y at the bottom edge */

/**
 * @brief XPT2046 operation status
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
 * @brief Touch state tracked by XPT2046_Update()
 */
typedef enum {
    XPT2046_STATE_RELEASED = 0,     /**< No contact */
    XPT2046_STATE_PRESSED,          /**< Contact started on this update */
    XPT2046_STATE_HELD              /**< Contact continued from a previous update */
} XPT2046_TouchState_t;

/**
 * @brief Mapped touch point
 */
typedef struct {
    uint16_t x;                     /**< X coordinate in display pixels */
    uint16_t y;                     /**< Y coordinate in display pixels */
    uint16_t pressure;              /**< Touch pressure, 0..XPT2046_MAX_PRESSURE */
    XPT2046_TouchState_t state;     /**< Touch state */
} XPT2046_TouchPoint_t;

/**
 * @brief One unmapped acquisition of all four channels
 */
typedef struct {
    uint16_t x;                     /**< Raw X position reading */
    uint16_t y;                     /**< Raw Y position reading */
    uint16_t z1;                    /**< Raw Z1 reading */
    uint16_t z2;                    /**< Raw Z2 reading */
} XPT2046_RawSample_t;

/**
 * @brief XPT2046 configuration
 */
typedef struct {
    GPIO_TypeDef *cs_port;          /**< Chip select port */
    uint16_t cs_pin;                /**< Chip select pin */
    GPIO_TypeDef *irq_port;         /**< PENIRQ port */
    uint16_t irq_pin;               /**< PENIRQ pin */
    uint16_t width;                 /**< Display width for coordinate mapping */
    uint16_t height;                /**< Display height for coordinate mapping */
    uint16_t raw_x_min;             /**< Raw X reported at the left edge */
    uint16_t raw_x_max;             /**< Raw X reported at the right edge */
    uint16_t raw_y_min;             /**< Raw Y reported at the top edge */
    uint16_t raw_y_max;             /**< Raw Y reported at the bottom edge */
    bool flip_x;                    /**< Mirror the X axis */
    bool flip_y;                    /**< Mirror the Y axis */
} XPT2046_Config_t;

/**
 * @brief XPT2046 handle
 */
typedef struct {
    XPT2046_Config_t config;        /**< Configuration */
    XPT2046_TouchPoint_t touch;     /**< Touch state maintained by XPT2046_Update() */
    bool initialized;               /**< Initialization status */
} XPT2046_Handle_t;

#ifdef __cplusplus
}
#endif

#endif /* XPT2046_TYPES_H */
