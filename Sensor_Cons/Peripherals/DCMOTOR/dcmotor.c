/**
 ******************************************************************************
 * @file    dcmotor.c
 * @brief   DC motor lifecycle and configuration
 ******************************************************************************
 */

#include "dcmotor_core.h"
#include "dcmotor_control.h"
#include "dcmotor_convert.h"
#include "dcmotor_pwm.h"
#include "log.h"
#include <string.h>

/**
 * @brief Factory settings: 20 kHz carrier, 1000 duty steps, coast on stop.
 */
DCMOTOR_Config_t DCMOTOR_GetDefaultConfig(void) {

    DCMOTOR_Config_t config = {.pwmFrequencyHz = DCMOTOR_DEFAULT_PWM_FREQUENCY_HZ,
                               .speedSteps = DCMOTOR_DEFAULT_SPEED_STEPS,
                               .stopMode = DCMOTOR_STOP_COAST};

    return config;
}

/**
 * @brief Bring up the bridge pins and PWM base, leaving the motor stopped.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Init(DCMOTOR_Handle_t *hmotor, const DCMOTOR_Pins_t *pins) {
    
    if (hmotor == NULL || pins == NULL) {
        return DCMOTOR_INVALID_PARAM;
    }

    memset(hmotor, 0, sizeof(*hmotor));
    hmotor->pins = *pins;
    hmotor->config = DCMOTOR_GetDefaultConfig();
    hmotor->direction = DCMOTOR_DIR_FORWARD;

    DCMOTOR_StatusTypeDef status = DCMOTOR_PWM_Init(hmotor);
    if (status != DCMOTOR_OK) {
        return status;
    }

    /* The control layer refuses to act on a handle that is not yet marked
       initialised, so the flag has to be set before the first command. */
    hmotor->isInitialized = true;

    return DCMOTOR_OK;
}

/**
 * @brief Stop the motor, release the bridge and mark the handle uninitialised.
 */
DCMOTOR_StatusTypeDef DCMOTOR_DeInit(DCMOTOR_Handle_t *hmotor) {
    DCMOTOR_CHECK_HANDLE(hmotor);

    (void)DCMOTOR_Coast(hmotor);
    DCMOTOR_PWM_DeInit(&hmotor->pins);
    hmotor->isInitialized = false;

    return DCMOTOR_OK;
}

/**
 * @brief Reprogram the carrier and duty resolution, stopping the motor first.
 */
DCMOTOR_StatusTypeDef DCMOTOR_Config(DCMOTOR_Handle_t *hmotor, const DCMOTOR_Config_t *config) {
    DCMOTOR_CHECK_HANDLE(hmotor);

    if (DCMOTOR_ValidateConfig(config) != DCMOTOR_OK) {
        return DCMOTOR_INVALID_PARAM;
    }

    /* Changing the prescaler under load would step the duty to an arbitrary
       fraction of the new period, so the drive is released first. */
    (void)DCMOTOR_Coast(hmotor);

    const DCMOTOR_Config_t previous = hmotor->config;
    hmotor->config = *config;

    DCMOTOR_StatusTypeDef status = DCMOTOR_PWM_Init(hmotor);
    if (status != DCMOTOR_OK) {
        hmotor->config = previous;
        log_error("DCMOTOR: configuration rejected by the timer");
        return status;
    }

    return DCMOTOR_OK;
}
