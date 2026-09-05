/**
 ******************************************************************************
 * @file    dcmotor_pwm.c
 * @brief   DC motor H-bridge transport - internal to the DCMOTOR driver
 ******************************************************************************
 */

#include "dcmotor_pwm.h"
#include "tim_pwm.h"
#include "gpio.h"
#include "log.h"

static bool DCMOTOR_PWM_ResolveAlternate(const TIM_TypeDef *instance, uint8_t *alternate) {

    if (instance == TIM1 || instance == TIM2) {
        *alternate = GPIO_AF1_TIM1;
    }
    else if (instance == TIM3 || instance == TIM4 || instance == TIM5) {
        *alternate = GPIO_AF2_TIM3;
    }
    else if (instance == TIM8 || instance == TIM9 || instance == TIM10 || instance == TIM11) {
        *alternate = GPIO_AF3_TIM8;
    }
    else if (instance == TIM12 || instance == TIM13 || instance == TIM14) {
        *alternate = GPIO_AF9_TIM12;
    }
    else {
        /* Leaving Alternate at 0 would map the pin to SYS, not to the timer. */
        return false;
    }

    return true;
}

static bool DCMOTOR_PWM_IsValidChannel(uint32_t channel) {

    if (channel == TIM_CHANNEL_1 || channel == TIM_CHANNEL_2) {
        return true;
    }

    if (channel == TIM_CHANNEL_3 || channel == TIM_CHANNEL_4) {
        return true;
    }

    return false;
}

static DCMOTOR_StatusTypeDef DCMOTOR_PWM_Validate(const DCMOTOR_Pins_t *pins, uint8_t *alternate) {

    if (pins->htim == NULL || pins->htim->Instance == NULL) {
        log_error("DCMOTOR: null timer");
        return DCMOTOR_INVALID_PARAM;
    }

    if (pins->enablePort == NULL || pins->enablePin == 0U) {
        log_error("DCMOTOR: invalid enable pin");
        return DCMOTOR_INVALID_PARAM;
    }

    if (pins->in1Port == NULL || pins->in1Pin == 0U || pins->in2Port == NULL ||
        pins->in2Pin == 0U) {
        log_error("DCMOTOR: invalid direction pins");
        return DCMOTOR_INVALID_PARAM;
    }

    if (!DCMOTOR_PWM_IsValidChannel(pins->channel)) {
        log_error("DCMOTOR: unsupported timer channel");
        return DCMOTOR_INVALID_PARAM;
    }

    if (!DCMOTOR_PWM_ResolveAlternate(pins->htim->Instance, alternate)) {
        log_error("DCMOTOR: timer has no known GPIO alternate function");
        return DCMOTOR_INVALID_PARAM;
    }

    return DCMOTOR_OK;
}

static HAL_StatusTypeDef DCMOTOR_PWM_InitDirectionPins(const DCMOTOR_Pins_t *pins) {
    GPIO_InitTypeDef gpioInit = {
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_LOW,
    };

    gpioInit.Pin = pins->in1Pin;
    if (GPIO_Driver_Pin_Init(pins->in1Port, &gpioInit) != HAL_OK) {
        return HAL_ERROR;
    }

    gpioInit.Pin = pins->in2Pin;
    return GPIO_Driver_Pin_Init(pins->in2Port, &gpioInit);
}

static HAL_StatusTypeDef DCMOTOR_PWM_StartTimer(const DCMOTOR_Pins_t *pins,
                                                const DCMOTOR_Config_t *config) {
    /* An unreachable carrier is reported here rather than silently accepted:
       unlike a servo the exact frequency does not matter, only that it starts. */
    HAL_StatusTypeDef status = TIM_PWM_InitHz(pins->htim, pins->htim->Instance,
                                              config->pwmFrequencyHz, config->speedSteps);

    /* Without this the channel keeps its reset output-compare mode and never
       produces a PWM waveform, however the compare register is written. */
    if (status == HAL_OK) {
        status = TIM_PWM_ConfigChannel(pins->htim, pins->channel, 0U, TIM_OCPOLARITY_HIGH);
    }

    if (status == HAL_OK) {
        status = TIM_PWM_Start(pins->htim, pins->channel);
    }

    return status;
}

/**
 * @brief Bring up the enable AF pin, the direction outputs and the PWM base.
 */
DCMOTOR_StatusTypeDef DCMOTOR_PWM_Init(DCMOTOR_Handle_t *hmotor) {
    uint8_t alternate = 0;
    DCMOTOR_StatusTypeDef status = DCMOTOR_PWM_Validate(&hmotor->pins, &alternate);

    if (status != DCMOTOR_OK) {
        return status;
    }

    if (DCMOTOR_PWM_InitDirectionPins(&hmotor->pins) != HAL_OK) {
        log_error("DCMOTOR: direction GPIO init failed");
        return DCMOTOR_ERROR;
    }

    /* Released before the carrier starts, so a stale bridge state cannot jerk
       the motor the moment the enable pin goes live. */
    DCMOTOR_PWM_ApplyStop(&hmotor->pins, DCMOTOR_STOP_COAST);

    GPIO_InitTypeDef gpioInit = {
        .Pin = hmotor->pins.enablePin,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_FREQ_HIGH,
        .Alternate = alternate,
    };

    if (GPIO_Driver_Pin_Init(hmotor->pins.enablePort, &gpioInit) != HAL_OK) {
        log_error("DCMOTOR: enable GPIO init failed");
        DCMOTOR_PWM_DeInit(&hmotor->pins);
        return DCMOTOR_ERROR;
    }

    if (DCMOTOR_PWM_StartTimer(&hmotor->pins, &hmotor->config) != HAL_OK) {
        log_error("DCMOTOR: PWM timer setup failed");
        DCMOTOR_PWM_DeInit(&hmotor->pins);
        return DCMOTOR_ERROR;
    }

    return DCMOTOR_OK;
}

/**
 * @brief Stop PWM and release the enable and direction pins.
 */
void DCMOTOR_PWM_DeInit(const DCMOTOR_Pins_t *pins) {

    if (pins->htim != NULL) {
        (void)TIM_PWM_Stop(pins->htim, pins->channel);
    }

    if (pins->enablePort != NULL) {
        (void)GPIO_Driver_Pin_DeInit(pins->enablePort, pins->enablePin);
    }

    if (pins->in1Port != NULL) {
        (void)GPIO_Driver_Pin_DeInit(pins->in1Port, pins->in1Pin);
    }

    if (pins->in2Port != NULL) {
        (void)GPIO_Driver_Pin_DeInit(pins->in2Port, pins->in2Pin);
    }
}

/**
 * @brief Drive the enable pin at duty of the configured resolution.
 */
void DCMOTOR_PWM_SetDuty(const DCMOTOR_Pins_t *pins, uint32_t duty) {
    TIM_PWM_SetDuty(pins->htim, pins->channel, duty);
}

/**
 * @brief Put the two direction inputs in the pattern for direction.
 */
void DCMOTOR_PWM_ApplyDirection(const DCMOTOR_Pins_t *pins, DCMOTOR_Direction_t direction) {

    if (direction == DCMOTOR_DIR_FORWARD) {
        HAL_GPIO_WritePin(pins->in1Port, pins->in1Pin, GPIO_PIN_SET);
        HAL_GPIO_WritePin(pins->in2Port, pins->in2Pin, GPIO_PIN_RESET);
    }
    else {
        HAL_GPIO_WritePin(pins->in1Port, pins->in1Pin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(pins->in2Port, pins->in2Pin, GPIO_PIN_SET);
    }
}

/**
 * @brief Put the two direction inputs in the pattern for stopMode.
 */
void DCMOTOR_PWM_ApplyStop(const DCMOTOR_Pins_t *pins, DCMOTOR_StopMode_t stopMode) {
    GPIO_PinState level = (stopMode == DCMOTOR_STOP_BRAKE) ? GPIO_PIN_SET : GPIO_PIN_RESET;

    HAL_GPIO_WritePin(pins->in1Port, pins->in1Pin, level);
    HAL_GPIO_WritePin(pins->in2Port, pins->in2Pin, level);
}
