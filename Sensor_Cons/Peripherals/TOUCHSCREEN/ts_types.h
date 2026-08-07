/**
 * @file ts_types.h
 * @brief Shared vocabulary for the STMPE811 touchscreen driver
 * @details Board wiring, status/gesture enumerations and the handle record.
 *          No behaviour lives here, so every touchscreen module can include it
 *          without depending on another module.
 */

#ifndef TS_TYPES_H
#define TS_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* Driver constants ----------------------------------------------------------*/
#define TS_TIMEOUT                      1000    /*!< I2C transfer timeout in ms */
#define TS_MAX_TOUCHES                  1       /*!< The STMPE811 panel is single touch */

/* Display mapping constants -------------------------------------------------*/
#define TS_DISPLAY_WIDTH                240
#define TS_DISPLAY_HEIGHT               320

/* Board wiring: touch interrupt line ---------------------------------------*/
#define TS_INT_PIN                      GPIO_PIN_15   /*!< PA15 - Touch INT */
#define TS_INT_GPIO_PORT                GPIOA
#define TS_INT_EXTI_IRQn                EXTI15_10_IRQn
#define TS_INT_NVIC_PRIORITY            0x0F          /*!< EXTI NVIC priority for touch wake */

/* Board wiring: I2C bus -----------------------------------------------------*/
#define TS_I2C                          I2C3
#define TS_I2C_CLK_ENABLE()             __HAL_RCC_I2C3_CLK_ENABLE()
#define TS_I2C_CLK_DISABLE()            __HAL_RCC_I2C3_CLK_DISABLE()
#define TS_I2C_SCL_PIN                  GPIO_PIN_8    /*!< PA8 */
#define TS_I2C_SCL_GPIO_PORT            GPIOA
#define TS_I2C_SDA_PIN                  GPIO_PIN_9    /*!< PC9 */
#define TS_I2C_SDA_GPIO_PORT            GPIOC

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Touchscreen status enumeration
 */
typedef enum {
    TS_OK = 0,                          /**< Operation completed successfully */
    TS_ERROR,                           /**< General error occurred */
    TS_BUSY,                            /**< Touchscreen is busy */
    TS_TIMEOUT_ERROR,                   /**< Operation timed out */
    TS_INVALID_PARAM,                   /**< Invalid parameter provided */
    TS_NOT_INITIALIZED,                 /**< Device not initialized */
    TS_COMMUNICATION_ERROR,             /**< I2C communication error */
    TS_DEVICE_NOT_FOUND                 /**< STMPE811 device not found */
} TS_StatusTypeDef;

/**
 * @brief Touch state enumeration
 */
typedef enum {
    TS_TOUCH_RELEASED = 0,              /**< No touch detected */
    TS_TOUCH_PRESSED,                   /**< Touch detected */
    TS_TOUCH_MOVING                     /**< Touch moving */
} TS_TouchStateTypeDef;

/**
 * @brief Touch gesture enumeration
 */
typedef enum {
    TS_GESTURE_NONE = 0,                /**< No gesture */
    TS_GESTURE_TAP,                     /**< Single tap */
    TS_GESTURE_DOUBLE_TAP,              /**< Double tap */
    TS_GESTURE_LONG_PRESS,              /**< Long press */
    TS_GESTURE_SWIPE_UP,                /**< Swipe up */
    TS_GESTURE_SWIPE_DOWN,              /**< Swipe down */
    TS_GESTURE_SWIPE_LEFT,              /**< Swipe left */
    TS_GESTURE_SWIPE_RIGHT              /**< Swipe right */
} TS_GestureTypeDef;

/**
 * @brief Touch point structure
 */
typedef struct {
    uint16_t X;                         /**< X coordinate */
    uint16_t Y;                         /**< Y coordinate */
    uint16_t Z;                         /**< Pressure (Z coordinate) */
    TS_TouchStateTypeDef State;         /**< Touch state */
    uint32_t Timestamp;                 /**< Touch timestamp */
} TS_TouchPointTypeDef;

/**
 * @brief Touch data structure
 */
typedef struct {
    uint8_t TouchCount;                          /**< Number of active touches */
    TS_TouchPointTypeDef Points[TS_MAX_TOUCHES]; /**< Touch points */
    TS_GestureTypeDef Gesture;                   /**< Detected gesture */
    uint32_t GestureTimestamp;                   /**< Gesture timestamp */
} TS_TouchDataTypeDef;

/**
 * @brief Calibration data structure
 * @details Raw ADC bounds of the panel; TS_Init seeds them with typical values
 *          and the application overwrites them with corner measurements.
 */
typedef struct {
    uint16_t MinX;                      /**< Raw X at one horizontal edge */
    uint16_t MaxX;                      /**< Raw X at the opposite horizontal edge */
    uint16_t MinY;                      /**< Raw Y at one vertical edge */
    uint16_t MaxY;                      /**< Raw Y at the opposite vertical edge */
    bool IsCalibrated;                  /**< False while the seeded defaults are in use */
} TS_CalibrationTypeDef;

/**
 * @brief Touchscreen configuration structure
 * @note  The STMPE811 sampling, settling and FIFO registers are written from a
 *        fixed sequence in the driver, so they are not exposed here.
 */
typedef struct {
    bool InterruptEnable;               /**< Interrupt enable */
} TS_ConfigTypeDef;

/**
 * @brief Touchscreen handle structure
 */
typedef struct {
    I2C_HandleTypeDef *hi2c;            /**< I2C handle */
    TS_ConfigTypeDef Config;            /**< Configuration */
    TS_CalibrationTypeDef Calibration;  /**< Calibration data */
    TS_TouchDataTypeDef TouchData;      /**< Current touch data */
    TS_TouchDataTypeDef PrevTouchData;  /**< Previous touch data */
    bool IsInitialized;                 /**< Initialization status */
    uint32_t LastTouchTime;             /**< Last touch timestamp */
    uint16_t FilterX;                   /**< Last reported X after smoothing */
    uint16_t FilterY;                   /**< Last reported Y after smoothing */
    void (*TouchCallback)(void);        /**< Touch detected callback */
    void (*ReleaseCallback)(void);      /**< Touch released callback */
    void (*GestureCallback)(TS_GestureTypeDef gesture); /**< Gesture callback */
} TS_HandleTypeDef;

/**
 * @brief Guard used by every entry point that talks to the controller
 * @param hts Touchscreen handle
 * @return TS_OK when the handle is usable
 */
static inline TS_StatusTypeDef TS_CheckReady(const TS_HandleTypeDef *hts)
{
    if (hts == NULL) {
        return TS_INVALID_PARAM;
    }
    if (!hts->IsInitialized) {
        return TS_NOT_INITIALIZED;
    }
    return TS_OK;
}

#ifdef __cplusplus
}
#endif

#endif /* TS_TYPES_H */
