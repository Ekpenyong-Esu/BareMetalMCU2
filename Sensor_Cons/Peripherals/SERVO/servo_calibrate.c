/**
  ******************************************************************************
  * @file    servo_calibrate.c
  * @brief   Servo travel-limit calibration
  ******************************************************************************
  */

#include "servo_calibrate.h"
#include "servo_control.h"
#include "servo_convert.h"
#include "servo_core.h"

/* A limit change can leave the resting angle outside the new range, which every
   later SERVO_SetAngle would then reject. */
static SERVO_StatusTypeDef SERVO_ClampToRange(SERVO_Handle_t *hservo)
{
    SERVO_Config_t *config = &hservo->config;

    if (config->defaultAngle < config->minAngle) {
        config->defaultAngle = config->minAngle;
    } else if (config->defaultAngle > config->maxAngle) {
        config->defaultAngle = config->maxAngle;
    }

    if (hservo->currentAngle < config->minAngle) {
        return SERVO_SetAngle(hservo, config->minAngle);
    }
    if (hservo->currentAngle > config->maxAngle) {
        return SERVO_SetAngle(hservo, config->maxAngle);
    }

    return SERVO_OK;
}

SERVO_StatusTypeDef SERVO_CalibrateMin(SERVO_Handle_t *hservo, uint16_t angle)
{
    SERVO_CHECK_HANDLE(hservo);

    if (angle >= hservo->config.maxAngle) {
        return SERVO_INVALID_PARAM;
    }

    hservo->config.minAngle = angle;

    return SERVO_ClampToRange(hservo);
}

SERVO_StatusTypeDef SERVO_CalibrateMax(SERVO_Handle_t *hservo, uint16_t angle)
{
    SERVO_CHECK_HANDLE(hservo);

    if (angle <= hservo->config.minAngle || angle > SERVO_MAX_ANGLE) {
        return SERVO_INVALID_PARAM;
    }

    hservo->config.maxAngle = angle;

    return SERVO_ClampToRange(hservo);
}

SERVO_StatusTypeDef SERVO_ResetCalibration(SERVO_Handle_t *hservo)
{
    SERVO_CHECK_HANDLE(hservo);

    hservo->config = SERVO_GetDefaultConfig();

    return SERVO_ClampToRange(hservo);
}
