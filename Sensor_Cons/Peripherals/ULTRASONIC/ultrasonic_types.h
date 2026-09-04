/**
 ******************************************************************************
 * @file    ultrasonic_types.h
 * @brief   Shared vocabulary for the ultrasonic distance sensor driver
 ******************************************************************************
 */

#ifndef ULTRASONIC_TYPES_H
#define ULTRASONIC_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/* Exported constants --------------------------------------------------------*/

/* Default configuration values */
#define ULTRASONIC_DEFAULT_MEASUREMENT_TIMEOUT 100U /* ms */
#define ULTRASONIC_DEFAULT_MIN_DISTANCE 20U         /* 2cm */
#define ULTRASONIC_DEFAULT_MAX_DISTANCE 4000U       /* 4m */
#define ULTRASONIC_DEFAULT_TEMPERATURE 20           /* 20 degC */

/* HC-SR04 timing constants */
#define ULTRASONIC_TRIGGER_PULSE_WIDTH_US 10U
#define ULTRASONIC_ECHO_TICK_HZ 1000000U /* capture timebase */
#define ULTRASONIC_ECHO_PERIOD 0xFFFFU   /* counter wrap point */
/* Spins allowed per microsecond before the trigger delay gives up on a timer
   that is not counting; generous enough never to cut a real pulse short. */
#define ULTRASONIC_DELAY_GUARD_LOOPS 1000U

/* Ambient temperature bounds accepted for the speed of sound correction */
#define ULTRASONIC_MIN_TEMPERATURE (-40)
#define ULTRASONIC_MAX_TEMPERATURE (85)

/* Sensor limits */
#define ULTRASONIC_ABS_MAX_DISTANCE 8000U /* beyond any HC-SR04 echo */

/* Exported types ------------------------------------------------------------*/

/**
 * @brief Ultrasonic sensor status enumeration
 */
typedef enum {
    ULTRASONIC_OK = 0,          /**< Operation completed successfully */
    ULTRASONIC_ERROR,           /**< General error occurred */
    ULTRASONIC_BUSY,            /**< Sensor is busy measuring */
    ULTRASONIC_TIMEOUT,         /**< Measurement timeout */
    ULTRASONIC_INVALID_PARAM,   /**< Invalid parameter provided */
    ULTRASONIC_NOT_INITIALIZED, /**< Driver not initialized */
    ULTRASONIC_OUT_OF_RANGE     /**< Echo outside the configured range */
} ULTRASONIC_StatusTypeDef;

/**
 * @brief Ultrasonic sensor configuration structure
 */
typedef struct {
    uint32_t measurementTimeout; /**< Echo measurement timeout in ms */
    uint16_t minDistance;        /**< Minimum measurable distance in mm */
    uint16_t maxDistance;        /**< Maximum measurable distance in mm */
    int8_t temperature;          /**< Ambient temperature in Celsius */
} ULTRASONIC_Config_t;

/**
 * @brief Ultrasonic sensor GPIO pins structure
 */
typedef struct {
    GPIO_TypeDef *triggerPort; /**< GPIO port for trigger pin */
    uint16_t triggerPin;       /**< GPIO pin for trigger signal */
    GPIO_TypeDef *echoPort;    /**< GPIO port for echo pin */
    uint16_t echoPin;          /**< GPIO pin for echo signal */
} ULTRASONIC_Pins_t;

/**
 * @brief Echo capture state machine
 */
typedef enum {
    ULTRASONIC_ECHO_IDLE = 0,    /**< No measurement in flight */
    ULTRASONIC_ECHO_WAIT_RISING, /**< Triggered, waiting for the echo to start */
    ULTRASONIC_ECHO_WAIT_FALLING /**< Echo started, waiting for it to end */
} ULTRASONIC_EchoState_t;

/**
 * @brief Ultrasonic sensor handle structure
 */
typedef struct {
    TIM_HandleTypeDef *htim;    /**< Timer handle for input capture */
    uint32_t channel;           /**< Timer channel for input capture */
    ULTRASONIC_Pins_t pins;     /**< GPIO pin configuration */
    ULTRASONIC_Config_t config; /**< Sensor configuration */

    volatile ULTRASONIC_EchoState_t echoState; /**< Capture state */
    volatile uint32_t echoStart;               /**< Capture value of the rising edge */
    volatile uint32_t echoTicks;               /**< Echo width of the last capture, in ticks */
    volatile bool measurementDone;             /**< Measurement completion flag */

    uint16_t lastDistance; /**< Last in-range distance in mm, 0 if none */
    bool isInitialized;    /**< Initialization status */
} ULTRASONIC_Handle_t;

/* Exported macros -----------------------------------------------------------*/

/**
 * @brief Common entry guard for every public entry point taking a handle.
 */
#define ULTRASONIC_CHECK_HANDLE(h)                                                                 \
    do {                                                                                           \
        if ((h) == NULL) {                                                                         \
            return ULTRASONIC_INVALID_PARAM;                                                       \
        }                                                                                          \
        if (!(h)->isInitialized) {                                                                 \
            return ULTRASONIC_NOT_INITIALIZED;                                                     \
        }                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* ULTRASONIC_TYPES_H */
