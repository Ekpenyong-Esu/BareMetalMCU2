/**
 ******************************************************************************
 * @file    servo.c
 * @brief   Servo lifecycle and configuration
 ******************************************************************************
 */

#include "servo_core.h"
#include "servo_control.h"
#include "servo_convert.h"
#include "servo_pwm.h"
#include "log.h"
#include <string.h>

/**
 * @brief Factory limits: 0-180°, 500-2500 us, default 90°.
 */
SERVO_Config_t SERVO_GetDefaultConfig(void) {
    SERVO_Config_t config = {.minAngle = SERVO_MIN_ANGLE,
                             .maxAngle = SERVO_MAX_ANGLE,
                             .minPulseWidth = SERVO_MIN_PULSE_WIDTH_US,
                             .maxPulseWidth = SERVO_MAX_PULSE_WIDTH_US,
                             .defaultAngle = SERVO_DEFAULT_ANGLE};

    return config;
}

/**
 * @brief Bring up PWM and move to default angle.
 */
SERVO_StatusTypeDef SERVO_Init(SERVO_Handle_t *hservo, TIM_HandleTypeDef *htim, uint32_t channel,
                               GPIO_TypeDef *gpioPort, uint16_t gpioPin) {
    SERVO_StatusTypeDef status = SERVO_OK;
    SERVO_Config_t defaultConfig;

    if (hservo == NULL || htim == NULL || htim->Instance == NULL || gpioPort == NULL) {
        return SERVO_INVALID_PARAM;
    }

    memset(hservo, 0, sizeof(SERVO_Handle_t));
    hservo->htim = htim;
    hservo->channel = channel;
    hservo->gpioPort = gpioPort;
    hservo->gpioPin = gpioPin;
    hservo->config = SERVO_GetDefaultConfig();

    status = SERVO_PWM_Init(hservo);
    if (status != SERVO_OK) {
        return status;
    }

    /* The control layer refuses to act on a handle that is not yet marked
       initialised, so the flag has to be set before the first move. */
    hservo->isInitialized = true;

    defaultConfig = SERVO_GetDefaultConfig();
    status = SERVO_Config(hservo, &defaultConfig);
    if (status != SERVO_OK) {
        hservo->isInitialized = false;
        SERVO_PWM_DeInit(hservo);
        log_error("SERVO: default configuration rejected");
        return status;
    }

    return SERVO_OK;
}

/**
 * @brief Stop PWM and mark handle uninitialised.
 */
SERVO_StatusTypeDef SERVO_DeInit(SERVO_Handle_t *hservo) {
    SERVO_CHECK_HANDLE(hservo);

    SERVO_PWM_DeInit(hservo);
    hservo->isInitialized = false;

    return SERVO_OK;
}

/**
 * @brief Replace travel limits and move to new default angle.
 */
SERVO_StatusTypeDef SERVO_Config(SERVO_Handle_t *hservo, const SERVO_Config_t *config) {
    SERVO_CHECK_HANDLE(hservo);

    if (SERVO_ValidateConfig(config) != SERVO_OK) {
        return SERVO_INVALID_PARAM;
    }

    hservo->config = *config;

    return SERVO_SetAngle(hservo, hservo->config.defaultAngle);
}
