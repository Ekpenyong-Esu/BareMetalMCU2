/**
 ******************************************************************************
 * @file    dcmotor_types.h
 * @brief   Brushed DC motor driver data definitions
 ******************************************************************************
 */

#ifndef DCMOTOR_TYPES_H
#define DCMOTOR_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/** @defgroup DCMOTOR_Drive_Specifications Drive Specifications
 * @{
 */
#define DCMOTOR_MIN_SPEED_PERCENT 0U      /*!< Motor released */
#define DCMOTOR_MAX_SPEED_PERCENT 100U    /*!< Full supply voltage across the motor */
#define DCMOTOR_DEFAULT_SPEED_PERCENT 50U /*!< Half speed */

/* A brushed motor is switched, not positioned, so the carrier is chosen to sit
   above hearing rather than to hit an exact frequency. */
#define DCMOTOR_DEFAULT_PWM_FREQUENCY_HZ 20000U /*!< 20 kHz, above the audible band */
#define DCMOTOR_DEFAULT_SPEED_STEPS 1000U       /*!< Duty resolution per period */

#define DCMOTOR_MIN_PWM_FREQUENCY_HZ 100U    /*!< Below this the motor cogs audibly */
#define DCMOTOR_MAX_PWM_FREQUENCY_HZ 100000U /*!< Above this bridge switching losses dominate */
#define DCMOTOR_MIN_SPEED_STEPS 100U         /*!< Coarsest useful duty resolution */
#define DCMOTOR_MAX_SPEED_STEPS 65535U       /*!< 16-bit auto-reload ceiling */
/** @} */

/**
 * @brief Brushed DC motor status enumeration
 */
typedef enum {
    DCMOTOR_OK = 0,          /*!< Operation completed successfully */
    DCMOTOR_ERROR,           /*!< General error occurred */
    DCMOTOR_INVALID_PARAM,   /*!< Invalid parameter provided */
    DCMOTOR_NOT_INITIALIZED, /*!< Motor not initialized */
    DCMOTOR_OUT_OF_RANGE     /*!< Speed out of valid range */
} DCMOTOR_StatusTypeDef;

/**
 * @brief Rotation direction enumeration
 * @note  Which way the shaft actually turns depends on how the two motor wires
 *        are landed on the bridge outputs; swap them to invert both directions.
 */
typedef enum {
    DCMOTOR_DIR_FORWARD = 0, /*!< IN1 high, IN2 low */
    DCMOTOR_DIR_REVERSE      /*!< IN1 low, IN2 high */
} DCMOTOR_Direction_t;

/**
 * @brief What the bridge does once the drive is released
 */
typedef enum {
    DCMOTOR_STOP_COAST = 0, /*!< Both inputs low: the motor freewheels to rest */
    DCMOTOR_STOP_BRAKE      /*!< Both inputs high: the windings short and resist motion */
} DCMOTOR_StopMode_t;

/**
 * @brief Brushed DC motor configuration structure
 */
typedef struct {
    uint32_t pwmFrequencyHz;      /*!< PWM carrier frequency in Hz */
    uint16_t speedSteps;          /*!< Duty resolution (compare values per period) */
    DCMOTOR_StopMode_t stopMode;  /*!< Bridge state used by DCMOTOR_Stop */
} DCMOTOR_Config_t;

/**
 * @brief One H-bridge half: an enable input carrying PWM and two direction inputs
 * @note  On an L298N these are ENA/IN1/IN2 for the first motor and ENB/IN3/IN4
 *        for the second. Remove the ENA jumper or the duty is ignored.
 */
typedef struct {
    TIM_HandleTypeDef *htim;   /*!< Timer handle driving the enable pin */
    uint32_t channel;          /*!< Timer PWM channel */
    GPIO_TypeDef *enablePort;  /*!< GPIO port for the enable (ENA/ENB) pin */
    uint16_t enablePin;        /*!< GPIO pin for the enable input */
    GPIO_TypeDef *in1Port;     /*!< GPIO port for direction input 1 */
    uint16_t in1Pin;           /*!< GPIO pin for direction input 1 */
    GPIO_TypeDef *in2Port;     /*!< GPIO port for direction input 2 */
    uint16_t in2Pin;           /*!< GPIO pin for direction input 2 */
} DCMOTOR_Pins_t;

/**
 * @brief Brushed DC motor handle structure
 */
typedef struct {
    DCMOTOR_Pins_t pins;            /*!< Bridge pin assignment */
    DCMOTOR_Config_t config;        /*!< Drive configuration */
    DCMOTOR_Direction_t direction;  /*!< Direction last commanded */
    uint8_t speedPercent;           /*!< Speed last commanded, 0..100 */
    bool isRunning;                 /*!< True while a non-zero speed is applied */
    bool isInitialized;             /*!< Initialization status */
} DCMOTOR_Handle_t;

/**
 * @brief Reject a NULL or uninitialised handle at every entry point.
 */
#define DCMOTOR_CHECK_HANDLE(hmotor)                                                               \
    do {                                                                                           \
        if ((hmotor) == NULL) {                                                                    \
            return DCMOTOR_INVALID_PARAM;                                                          \
        }                                                                                          \
        if (!(hmotor)->isInitialized) {                                                            \
            return DCMOTOR_NOT_INITIALIZED;                                                        \
        }                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* DCMOTOR_TYPES_H */
