/**
 ******************************************************************************
 * @file    servo_control.c
 * @brief   Servo positioning commands
 ******************************************************************************
 */

#include "servo_control.h"
#include "servo_convert.h"
#include "servo_pwm.h"
#include "stm32f4xx_hal.h"

SERVO_StatusTypeDef SERVO_SetAngle(SERVO_Handle_t *hservo, uint16_t angle) {
    SERVO_CHECK_HANDLE(hservo);

    if (!SERVO_IsValidAngle(angle, &hservo->config)) {
        return SERVO_OUT_OF_RANGE;
    }

    uint16_t pulseWidth = SERVO_AngleToPulseWidth(angle, &hservo->config);

    SERVO_StatusTypeDef status = SERVO_PWM_SetPulseWidth(hservo, pulseWidth);
    if (status == SERVO_OK) {
        hservo->currentAngle = angle;
        hservo->currentPulse = pulseWidth;
    }

    return status;
}

SERVO_StatusTypeDef SERVO_SetPulseWidth(SERVO_Handle_t *hservo, uint16_t pulseWidth) {
    SERVO_CHECK_HANDLE(hservo);

    if (!SERVO_IsValidPulseWidth(pulseWidth, &hservo->config)) {
        return SERVO_INVALID_PARAM;
    }

    SERVO_StatusTypeDef status = SERVO_PWM_SetPulseWidth(hservo, pulseWidth);
    if (status == SERVO_OK) {
        hservo->currentPulse = pulseWidth;
        /* Keep the reported angle consistent with the pulse actually driven. */
        hservo->currentAngle = SERVO_PulseWidthToAngle(pulseWidth, &hservo->config);
    }

    return status;
}

uint16_t SERVO_GetAngle(const SERVO_Handle_t *hservo) {
    return (hservo != NULL) ? hservo->currentAngle : 0U;
}

uint16_t SERVO_GetPulseWidth(const SERVO_Handle_t *hservo) {
    return (hservo != NULL) ? hservo->currentPulse : 0U;
}

SERVO_StatusTypeDef SERVO_MoveToMin(SERVO_Handle_t *hservo) {
    SERVO_CHECK_HANDLE(hservo);

    return SERVO_SetAngle(hservo, hservo->config.minAngle);
}

SERVO_StatusTypeDef SERVO_MoveToMax(SERVO_Handle_t *hservo) {
    SERVO_CHECK_HANDLE(hservo);

    return SERVO_SetAngle(hservo, hservo->config.maxAngle);
}

SERVO_StatusTypeDef SERVO_MoveToCenter(SERVO_Handle_t *hservo) {
    SERVO_CHECK_HANDLE(hservo);

    uint16_t center =
        (uint16_t)(((uint32_t)hservo->config.minAngle + hservo->config.maxAngle) / 2U);

    return SERVO_SetAngle(hservo, center);
}

SERVO_StatusTypeDef SERVO_Sweep(SERVO_Handle_t *hservo, uint16_t speed) {
    SERVO_StatusTypeDef status = SERVO_OK;
    uint16_t minAngle = 0;
    uint16_t maxAngle = 0;
    uint16_t angle = 0;

    SERVO_CHECK_HANDLE(hservo);

    minAngle = hservo->config.minAngle;
    maxAngle = hservo->config.maxAngle;

    for (angle = minAngle; angle <= maxAngle; angle++) {
        status = SERVO_SetAngle(hservo, angle);
        if (status != SERVO_OK) {
            return status;
        }
        HAL_Delay(speed);
    }

    /* Counting down with an unsigned loop variable cannot use `angle >= minAngle`:
       at minAngle == 0 the decrement wraps to 65535 and the loop never ends. */
    for (angle = maxAngle;; angle--) {
        status = SERVO_SetAngle(hservo, angle);
        if (status != SERVO_OK) {
            return status;
        }
        HAL_Delay(speed);

        if (angle == minAngle) {
            break;
        }
    }

    return SERVO_OK;
}
