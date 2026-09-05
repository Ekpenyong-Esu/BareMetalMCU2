/**
 ******************************************************************************
 * @file    joystick_types.h
 * @brief   Analog two-axis joystick driver data definitions
 ******************************************************************************
 */

#ifndef JOYSTICK_TYPES_H
#define JOYSTICK_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "adc_types.h"
#include "stm32f4xx.h"
#include <stdbool.h>
#include <stdint.h>

/** @defgroup JOYSTICK_Travel_Specifications Travel Specifications
 * @{
 */
#define JOYSTICK_PERCENT_SCALE 100U /*!< Full deflection, either side of centre */
#define JOYSTICK_MIN_PERCENT (-100) /*!< Hard left / fully down */
#define JOYSTICK_MAX_PERCENT (100)  /*!< Hard right / fully up */

/* A resting stick wanders by a few counts, so a band around centre is reported
   as zero rather than as a slow creep the actuator would act on. */
#define JOYSTICK_DEFAULT_DEADZONE_PERCENT 10U /*!< Ignored band around centre */
#define JOYSTICK_MAX_DEADZONE_PERCENT 50U     /*!< Beyond this half the travel is dead */

#define JOYSTICK_DEFAULT_DIRECTION_THRESHOLD_PERCENT 50U /*!< Deflection that counts as a push */

#define JOYSTICK_DEFAULT_RAW_MAX 4095U /*!< 12-bit ADC full scale */
#define JOYSTICK_MIN_RAW_MAX 255U      /*!< Coarsest resolution worth scaling */

#define JOYSTICK_CALIBRATION_SAMPLES 16U /*!< Readings averaged to find centre */
/** @} */

/**
 * @brief Joystick status enumeration
 */
typedef enum {
    JOYSTICK_OK = 0,          /*!< Operation completed successfully */
    JOYSTICK_ERROR,           /*!< General error occurred */
    JOYSTICK_INVALID_PARAM,   /*!< Invalid parameter provided */
    JOYSTICK_NOT_INITIALIZED, /*!< Joystick not initialized */
} JOYSTICK_StatusTypeDef;

/**
 * @brief Discrete direction, for callers that want a decision rather than a number
 */
typedef enum {
    JOYSTICK_DIR_CENTER = 0, /*!< Inside the direction threshold on both axes */
    JOYSTICK_DIR_UP,         /*!< Y pushed positive */
    JOYSTICK_DIR_DOWN,       /*!< Y pushed negative */
    JOYSTICK_DIR_LEFT,       /*!< X pushed negative */
    JOYSTICK_DIR_RIGHT       /*!< X pushed positive */
} JOYSTICK_Direction_t;

/**
 * @brief Joystick configuration structure
 */
typedef struct {
    uint16_t rawMax;                    /*!< ADC full-scale count */
    uint8_t deadzonePercent;            /*!< Band around centre reported as zero */
    uint8_t directionThresholdPercent;  /*!< Deflection needed to name a direction */
    bool invertX;                       /*!< Flip the X sign to match the mounting */
    bool invertY;                       /*!< Flip the Y sign to match the mounting */
    bool buttonActiveLow;               /*!< True when the switch pulls the pin to ground */
} JOYSTICK_Config_t;

/**
 * @brief Where the joystick is wired
 * @note  The ADC handle must already be initialised; this driver reads channels
 *        but never brings the ADC up. Leave buttonPort NULL if the module has
 *        no push switch or you do not need it.
 */
typedef struct {
    ADC_HandleStruct *hadc;   /*!< Initialised ADC handle used for both axes */
    uint32_t xChannel;        /*!< ADC channel wired to VRx */
    uint32_t yChannel;        /*!< ADC channel wired to VRy */
    GPIO_TypeDef *buttonPort; /*!< GPIO port for the push switch, NULL if unused */
    uint16_t buttonPin;       /*!< GPIO pin for the push switch */
} JOYSTICK_Pins_t;

/**
 * @brief Resting position measured by JOYSTICK_Calibrate
 */
typedef struct {
    uint16_t xCenter; /*!< Raw count with the stick at rest on X */
    uint16_t yCenter; /*!< Raw count with the stick at rest on Y */
} JOYSTICK_Calibration_t;

/**
 * @brief One complete reading
 */
typedef struct {
    int8_t xPercent;      /*!< -100 (left) to +100 (right), 0 inside the deadzone */
    int8_t yPercent;      /*!< -100 (down) to +100 (up), 0 inside the deadzone */
    bool buttonPressed;   /*!< True while the stick is pressed down */
} JOYSTICK_Position_t;

/**
 * @brief Joystick handle structure
 */
typedef struct {
    JOYSTICK_Pins_t pins;               /*!< Wiring */
    JOYSTICK_Config_t config;           /*!< Scaling and orientation */
    JOYSTICK_Calibration_t calibration; /*!< Measured rest position */
    bool isInitialized;                 /*!< Initialization status */
} JOYSTICK_Handle_t;

/**
 * @brief Reject a NULL or uninitialised handle at every entry point.
 */
#define JOYSTICK_CHECK_HANDLE(hjoy)                                                                \
    do {                                                                                           \
        if ((hjoy) == NULL) {                                                                      \
            return JOYSTICK_INVALID_PARAM;                                                         \
        }                                                                                          \
        if (!(hjoy)->isInitialized) {                                                              \
            return JOYSTICK_NOT_INITIALIZED;                                                       \
        }                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* JOYSTICK_TYPES_H */
