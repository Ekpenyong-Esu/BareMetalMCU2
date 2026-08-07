/**
  ******************************************************************************
  * @file    servo_types.h
  * @brief   Servo motor driver data definitions
  ******************************************************************************
  */

#ifndef SERVO_TYPES_H
#define SERVO_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/** @defgroup SERVO_Motor_Specifications Motor Specifications
 * @{
 */
#define SERVO_MIN_ANGLE              0       /*!< Minimum angle in degrees */
#define SERVO_MAX_ANGLE              180     /*!< Maximum angle in degrees */
#define SERVO_DEFAULT_ANGLE          90      /*!< Default angle in degrees */

#define SERVO_MIN_PULSE_WIDTH_US     500     /*!< Minimum pulse width in microseconds */
#define SERVO_MAX_PULSE_WIDTH_US     2500    /*!< Maximum pulse width in microseconds */
#define SERVO_DEFAULT_PULSE_WIDTH_US 1500    /*!< Default pulse width in microseconds */

#define SERVO_PWM_FREQUENCY_HZ       50      /*!< Standard servo PWM frequency (20ms period) */
#define SERVO_PWM_PERIOD_US          20000   /*!< PWM period in microseconds (20ms) */
/** @} */

/**
 * @brief Servo motor status enumeration
 */
typedef enum {
    SERVO_OK = 0,              /*!< Operation completed successfully */
    SERVO_ERROR,               /*!< General error occurred */
    SERVO_BUSY,                /*!< Servo is busy */
    SERVO_TIMEOUT,             /*!< Operation timed out */
    SERVO_INVALID_PARAM,       /*!< Invalid parameter provided */
    SERVO_NOT_INITIALIZED,     /*!< Servo not initialized */
    SERVO_OUT_OF_RANGE         /*!< Angle out of valid range */
} SERVO_StatusTypeDef;

/**
 * @brief Servo motor configuration structure
 */
typedef struct {
    uint16_t minAngle;         /*!< Minimum angle in degrees */
    uint16_t maxAngle;         /*!< Maximum angle in degrees */
    uint16_t minPulseWidth;    /*!< Minimum pulse width in microseconds */
    uint16_t maxPulseWidth;    /*!< Maximum pulse width in microseconds */
    uint16_t defaultAngle;     /*!< Default angle in degrees */
} SERVO_Config_t;

/**
 * @brief Servo motor handle structure
 */
typedef struct {
    TIM_HandleTypeDef *htim;   /*!< Timer handle for PWM generation */
    uint32_t channel;          /*!< Timer PWM channel */
    GPIO_TypeDef *gpioPort;    /*!< GPIO port for PWM pin */
    uint16_t gpioPin;          /*!< GPIO pin for PWM signal */
    SERVO_Config_t config;     /*!< Servo configuration */
    uint16_t currentAngle;     /*!< Current angle in degrees */
    uint16_t currentPulse;     /*!< Current pulse width in microseconds */
    bool isInitialized;        /*!< Initialization status */
} SERVO_Handle_t;

/**
 * @brief Reject a NULL or uninitialised handle at every entry point.
 */
#define SERVO_CHECK_HANDLE(hservo)                     \
    do {                                               \
        if ((hservo) == NULL) {                        \
            return SERVO_INVALID_PARAM;                \
        }                                              \
        if (!(hservo)->isInitialized) {                \
            return SERVO_NOT_INITIALIZED;              \
        }                                              \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* SERVO_TYPES_H */
