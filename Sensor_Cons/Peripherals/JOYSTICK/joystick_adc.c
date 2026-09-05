/**
 ******************************************************************************
 * @file    joystick_adc.c
 * @brief   Joystick hardware access - internal to the JOYSTICK driver
 ******************************************************************************
 */

#include "joystick_adc.h"
#include "adc_convert.h"
#include "gpio.h"
#include "log.h"

static JOYSTICK_StatusTypeDef JOYSTICK_ADC_Validate(const JOYSTICK_Pins_t *pins) {

    if (pins == NULL) {
        log_error("JOYSTICK: pins are NULL");
        return JOYSTICK_INVALID_PARAM;
    }

    if (pins->hadc == NULL) {
        log_error("JOYSTICK: ADC handle is NULL");
        return JOYSTICK_INVALID_PARAM;
    }

    if (ADC_CheckReady(pins->hadc) != HAL_OK) {
        log_error("JOYSTICK: ADC must be initialised before the joystick");
        return JOYSTICK_INVALID_PARAM;
    }

    if (pins->xChannel == pins->yChannel) {
        log_error("JOYSTICK: both axes are on the same ADC channel");
        return JOYSTICK_INVALID_PARAM;
    }

    return JOYSTICK_OK;
}

/* An open switch leaves the pin floating, so the pull has to sit opposite the
   level the switch drives. */
static JOYSTICK_StatusTypeDef JOYSTICK_ADC_InitButtonPin(const JOYSTICK_Pins_t *pins,
                                                         bool activeLow) {

    if (pins->buttonPort == NULL) {
        return JOYSTICK_OK;
    }

    GPIO_InitTypeDef gpioInit = {0};

    gpioInit.Pin = pins->buttonPin;
    gpioInit.Mode = GPIO_MODE_INPUT;
    gpioInit.Pull = GPIO_PULLDOWN;
    gpioInit.Speed = GPIO_SPEED_FREQ_LOW;

    if (activeLow) {
        gpioInit.Pull = GPIO_PULLUP;
    }

    if (GPIO_Driver_Pin_Init(pins->buttonPort, &gpioInit) != HAL_OK) {
        log_error("JOYSTICK: button GPIO init failed");
        return JOYSTICK_ERROR;
    }

    return JOYSTICK_OK;
}

/**
 * @brief Check the wiring and bring up the push-switch pin.
 */
JOYSTICK_StatusTypeDef JOYSTICK_ADC_Init(const JOYSTICK_Pins_t *pins,
                                         const JOYSTICK_Config_t *config) {

    if (config == NULL) {
        return JOYSTICK_INVALID_PARAM;
    }

    JOYSTICK_StatusTypeDef status = JOYSTICK_ADC_Validate(pins);

    if (status != JOYSTICK_OK) {
        return status;
    }

    return JOYSTICK_ADC_InitButtonPin(pins, config->buttonActiveLow);
}

/**
 * @brief Release the push-switch pin.
 */
void JOYSTICK_ADC_DeInit(const JOYSTICK_Pins_t *pins) {

    if (pins == NULL || pins->buttonPort == NULL) {
        return;
    }

    (void)GPIO_Driver_Pin_DeInit(pins->buttonPort, pins->buttonPin);
}

/**
 * @brief Read one axis.
 */
JOYSTICK_StatusTypeDef JOYSTICK_ADC_ReadAxis(const JOYSTICK_Pins_t *pins, uint32_t channel,
                                             uint16_t *raw) {

    if (pins == NULL || pins->hadc == NULL || raw == NULL) {
        return JOYSTICK_INVALID_PARAM;
    }

    uint32_t value = 0;

    if (ADC_ReadChannel(pins->hadc, channel, &value) != HAL_OK) {
        log_error("JOYSTICK: ADC read failed");
        return JOYSTICK_ERROR;
    }

    *raw = (uint16_t)value;

    return JOYSTICK_OK;
}

/**
 * @brief Read the push switch.
 */
bool JOYSTICK_ADC_ReadButton(const JOYSTICK_Pins_t *pins, bool activeLow) {

    if (pins == NULL || pins->buttonPort == NULL) {
        return false;
    }

    GPIO_PinState state = HAL_GPIO_ReadPin(pins->buttonPort, pins->buttonPin);

    if (activeLow) {
        return (state == GPIO_PIN_RESET);
    }

    return (state == GPIO_PIN_SET);
}
