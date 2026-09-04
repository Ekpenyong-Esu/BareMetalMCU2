/**
 ******************************************************************************
 * @file    stepper_types.h
 * @brief   Shared vocabulary for the stepper motor driver
 ******************************************************************************
 */

#ifndef STEPPER_TYPES_H
#define STEPPER_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/** @defgroup STEPPER_Motor_Specifications Motor Specifications
 * @{
 */
#define STEPPER_MAX_SPEED_RPM 1000U        /*!< Maximum speed in RPM */
#define STEPPER_MIN_SPEED_RPM 1U           /*!< Minimum speed in RPM */
#define STEPPER_DEFAULT_STEPS_PER_REV 200U /*!< Default steps per revolution */
/** @} */

/** Step interval limits, in microseconds */
#define STEPPER_MIN_DELAY_US 100U
#define STEPPER_MAX_DELAY_US 100000U

/** Number of coils driven by the sequence tables */
#define STEPPER_COIL_COUNT 4U

/**
 * @brief Stepper motor status enumeration
 */
typedef enum {
    STEPPER_OK = 0,          /*!< Operation completed successfully */
    STEPPER_ERROR,           /*!< General error occurred */
    STEPPER_BUSY,            /*!< Motor is busy */
    STEPPER_INVALID_PARAM,   /*!< Invalid parameter provided */
    STEPPER_NOT_INITIALIZED, /*!< Motor not initialized */
    STEPPER_TIMEOUT,         /*!< Operation timed out */
    STEPPER_LIMIT_REACHED    /*!< Movement limit reached */
} STEPPER_StatusTypeDef;

/**
 * @brief Stepper motor direction enumeration
 */
typedef enum {
    STEPPER_DIR_CW = 0, /*!< Clockwise direction */
    STEPPER_DIR_CCW     /*!< Counter-clockwise direction */
} STEPPER_Direction_t;

/**
 * @brief Stepper motor step mode enumeration
 */
typedef enum {
    STEPPER_MODE_FULL_STEP = 0, /*!< Full step mode */
    STEPPER_MODE_HALF_STEP,     /*!< Half step mode */
    STEPPER_MODE_WAVE_DRIVE     /*!< Wave drive mode */
} STEPPER_StepMode_t;

/**
 * @brief Stepper motor configuration structure
 */
typedef struct {
    uint16_t stepsPerRevolution; /*!< Steps per revolution */
    uint16_t maxSpeedRPM;        /*!< Speed ceiling enforced by the motion calls */
    STEPPER_StepMode_t stepMode; /*!< Step mode */
} STEPPER_Config_t;

/**
 * @brief Stepper motor GPIO pin configuration
 */
typedef struct {
    GPIO_TypeDef *port1; /*!< GPIO port for coil 1 */
    uint16_t pin1;       /*!< GPIO pin for coil 1 */
    GPIO_TypeDef *port2; /*!< GPIO port for coil 2 */
    uint16_t pin2;       /*!< GPIO pin for coil 2 */
    GPIO_TypeDef *port3; /*!< GPIO port for coil 3 */
    uint16_t pin3;       /*!< GPIO pin for coil 3 */
    GPIO_TypeDef *port4; /*!< GPIO port for coil 4 */
    uint16_t pin4;       /*!< GPIO pin for coil 4 */
} STEPPER_Pins_t;

/**
 * @brief Stepper motor handle structure
 */
typedef struct {
    TIM_HandleTypeDef *htim;       /*!< Timer used as the microsecond time base */
    STEPPER_Pins_t pins;           /*!< GPIO pin configuration */
    STEPPER_Config_t config;       /*!< Motor configuration */
    STEPPER_Direction_t direction; /*!< Current direction */
    int32_t currentPosition;       /*!< Signed step count; CCW moves it below zero */
    bool isRunning;                /*!< Motor running status */
    bool isInitialized;            /*!< Initialization status */
    uint32_t stepDelay;            /*!< Delay between steps in microseconds */
    uint8_t currentStep;           /*!< Current index into the coil sequence */
} STEPPER_Handle_t;

/** Reject a handle that is NULL or was never initialized */
#define STEPPER_CHECK_HANDLE(hstep)                                                                \
    do {                                                                                           \
        if ((hstep) == NULL) {                                                                     \
            return STEPPER_INVALID_PARAM;                                                          \
        }                                                                                          \
        if (!(hstep)->isInitialized) {                                                             \
            return STEPPER_NOT_INITIALIZED;                                                        \
        }                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_TYPES_H */
