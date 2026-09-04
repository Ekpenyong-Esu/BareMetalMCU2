/**
 * @file ir.c
 * @brief IR driver lifecycle, carrier configuration and status
 */

/* Includes ------------------------------------------------------------------*/
#include "ir_core.h"
#include "ir_receive.h"
#include "ir_timing.h"

/* Private define ------------------------------------------------------------*/

/** Percent denominator used for the carrier duty cycle */
#define IR_DUTY_CYCLE_SCALE 100U

/** Capture timer free-runs over its full 32-bit range; edges are timestamped */
#define IR_CAPTURE_PERIOD_FREE_RUN 0xFFFFFFFFU

/* Private function prototypes -----------------------------------------------*/
static uint32_t IR_GetTimerClock(void);
static HAL_StatusTypeDef IR_ConfigureCaptureTimer(IR_Handle_t *handle);

/* Public functions ----------------------------------------------------------*/

HAL_StatusTypeDef IR_Init(IR_Handle_t *handle, TIM_HandleTypeDef *htimCarrier,
                          TIM_HandleTypeDef *htimCapture, uint32_t txChannel, uint32_t rxChannel,
                          const IR_Config_t *config) {
    HAL_StatusTypeDef status = HAL_OK;

    if (handle == NULL || htimCarrier == NULL || htimCapture == NULL || config == NULL) {
        return HAL_ERROR;
    }

    handle->htimCarrier = htimCarrier;
    handle->htimCapture = htimCapture;
    handle->txChannel = txChannel;
    handle->rxChannel = rxChannel;
    handle->config = *config;

    IR_TimingInit();
    if (!IR_TimingIsRunning()) {
        return HAL_ERROR;
    }

    handle->state = IR_STATE_IDLE;
    handle->errorCode = IR_ERROR_NONE;
    handle->initialized = false;
    handle->rxIndex = 0;
    handle->lastCaptureTime = 0;
    handle->rxHasReference = false;
    handle->txIndex = 0;
    handle->txCount = 0;
    handle->captureTickFreq = IR_CAPTURE_TICK_FREQ;
    handle->eventCallback = NULL;

    handle->rxFrame.valid = false;
    handle->rxFrame.repeat = false;

    status = IR_ConfigureCaptureTimer(handle);
    if (status != HAL_OK) {
        handle->errorCode = IR_ERROR_TIMER;
        return status;
    }

    /* The carrier setup below goes through the public API, which rejects an
       uninitialised handle, so flag the handle as ready first. */
    handle->initialized = true;

    status = IR_ConfigureCarrier(handle, config->carrierFreq, config->dutyCycle);
    if (status != HAL_OK) {
        handle->errorCode = IR_ERROR_INIT;
        handle->initialized = false;
        return status;
    }

    return HAL_OK;
}

HAL_StatusTypeDef IR_DeInit(IR_Handle_t *handle) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    (void)IR_StopReceive(handle);

    (void)HAL_TIM_PWM_Stop(handle->htimCarrier, handle->txChannel);

    handle->state = IR_STATE_IDLE;
    handle->initialized = false;

    return HAL_OK;
}

HAL_StatusTypeDef IR_SetEventCallback(IR_Handle_t *handle,
                                      void (*callback)(IR_Event_t event, IR_Frame_t *frame)) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->eventCallback = callback;

    return HAL_OK;
}

HAL_StatusTypeDef IR_ConfigureCarrier(IR_Handle_t *handle, uint32_t frequency, uint8_t dutyCycle) {
    TIM_OC_InitTypeDef sConfigOC = {0};
    uint32_t timerClock = 0;
    uint32_t period = 0;

    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    if (frequency == 0U || dutyCycle == 0U || dutyCycle >= IR_DUTY_CYCLE_SCALE) {
        handle->errorCode = IR_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    timerClock = IR_GetTimerClock();
    if (frequency > timerClock) {
        handle->errorCode = IR_ERROR_INVALID_PARAM;
        return HAL_ERROR;
    }

    period = (timerClock / frequency) - 1U;

    /* No prescaler: the carrier is in the tens of kHz, so the full timer clock
       gives the finest duty cycle resolution the counter can hold. */
    handle->htimCarrier->Init.Prescaler = 0U;
    handle->htimCarrier->Init.Period = period;

    if (HAL_TIM_PWM_Init(handle->htimCarrier) != HAL_OK) {
        handle->errorCode = IR_ERROR_TIMER;
        return HAL_ERROR;
    }

    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = (period * dutyCycle) / IR_DUTY_CYCLE_SCALE;
    sConfigOC.OCPolarity = handle->config.invertSignal ? TIM_OCPOLARITY_LOW : TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;

    if (HAL_TIM_PWM_ConfigChannel(handle->htimCarrier, &sConfigOC, handle->txChannel) != HAL_OK) {
        handle->errorCode = IR_ERROR_TIMER;
        return HAL_ERROR;
    }

    handle->config.carrierFreq = frequency;
    handle->config.dutyCycle = dutyCycle;

    return HAL_OK;
}

HAL_StatusTypeDef IR_SetTolerance(IR_Handle_t *handle, uint16_t tolerance) {
    if (IR_ValidateHandle(handle) != HAL_OK) {
        return HAL_ERROR;
    }

    handle->config.tolerance = tolerance;

    return HAL_OK;
}

IR_State_t IR_GetState(IR_Handle_t *handle) {
    if (handle == NULL) {
        return IR_STATE_ERROR;
    }

    return handle->state;
}

uint32_t IR_GetError(IR_Handle_t *handle) {
    if (handle == NULL) {
        return IR_ERROR_INIT;
    }

    return handle->errorCode;
}

HAL_StatusTypeDef IR_ClearError(IR_Handle_t *handle) {
    if (handle == NULL) {
        return HAL_ERROR;
    }

    handle->errorCode = IR_ERROR_NONE;

    if (handle->state == IR_STATE_ERROR) {
        handle->state = IR_STATE_IDLE;
    }

    return HAL_OK;
}

HAL_StatusTypeDef IR_ValidateHandle(const IR_Handle_t *handle) {
    if (handle == NULL || !handle->initialized) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

void IR_NotifyEvent(IR_Handle_t *handle, IR_Event_t event, IR_Frame_t *frame) {
    if (handle->eventCallback != NULL) {
        handle->eventCallback(event, frame);
    }
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Clock feeding the timers used by this driver
 * @note  APB1 timers run at twice PCLK1 whenever the APB1 prescaler is not 1.
 * @return uint32_t: Timer input clock in Hz
 */
static uint32_t IR_GetTimerClock(void) {
    return HAL_RCC_GetPCLK1Freq() * 2U;
}

/**
 * @brief Prescale the capture timer to one tick per microsecond
 * @param handle: Pointer to IR handle structure
 * @return HAL_StatusTypeDef: HAL status
 */
static HAL_StatusTypeDef IR_ConfigureCaptureTimer(IR_Handle_t *handle) {
    const uint32_t timerClock = IR_GetTimerClock();
    const uint32_t prescaler = timerClock / IR_CAPTURE_TICK_FREQ;

    if (prescaler == 0U) {
        return HAL_ERROR;
    }

    handle->htimCapture->Init.Prescaler = prescaler - 1U;
    handle->htimCapture->Init.Period = IR_CAPTURE_PERIOD_FREE_RUN;
    handle->htimCapture->Init.CounterMode = TIM_COUNTERMODE_UP;

    /* The decoders convert captured ticks with this rate, not with the raw
       timer clock, so record what the prescaler actually produced. */
    handle->captureTickFreq = timerClock / prescaler;

    return HAL_TIM_IC_Init(handle->htimCapture);
}
