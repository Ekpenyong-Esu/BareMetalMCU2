/**
 ******************************************************************************
 * @file    dht_bus.c
 * @brief   DHT one-wire transport - internal to the DHT driver
 ******************************************************************************
 */

#include "dht_bus.h"

#include "dht_timing.h"
#include "gpio.h"

/* A data bit longer than this is a '1' (0-bits are ~27us, 1-bits ~70us) */
#define DHT_BIT_ONE_THRESHOLD_US 40U

/* Bus released high for 20..40 us before the sensor answers */
#define DHT_START_RELEASE_US 30U

/* Timeouts for each edge, all a little longer than the nominal pulse */
#define DHT_RESPONSE_TIMEOUT_US 100U /* sensor's 80 us low / 80 us high preamble */
#define DHT_BIT_LOW_TIMEOUT_US 70U   /* ~50 us low that starts every bit */
#define DHT_BIT_HIGH_TIMEOUT_US 100U /* 27 us ('0') or 70 us ('1') high that encodes it */

#define DHT_BITS_PER_BYTE 8U

static void DHT_Bus_SetOutput(GPIO_TypeDef *port, uint16_t pin) {
    GPIO_InitTypeDef init = {0};

    init.Pin = pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_Driver_Pin_Init(port, &init);
}

static void DHT_Bus_SetInput(GPIO_TypeDef *port, uint16_t pin) {
    GPIO_InitTypeDef init = {0};

    init.Pin = pin;
    init.Mode = GPIO_MODE_INPUT;
    /* Keeps the line defined if the module has no external pull-up. */
    init.Pull = GPIO_PULLUP;
    GPIO_Driver_Pin_Init(port, &init);
}

static bool DHT_Bus_WaitLevel(GPIO_TypeDef *port, uint16_t pin, GPIO_PinState level,
                              uint32_t timeoutUs) {
    uint32_t start = DHT_Timing_Now();

    while (DHT_Timing_ElapsedUs(start) < timeoutUs) {
        if (GPIO_Driver_ReadPin(port, pin) == level) {
            return true;
        }
    }

    return false;
}

/** @brief The 80 us low / 80 us high the sensor sends before the data */
static bool DHT_Bus_AwaitResponse(GPIO_TypeDef *port, uint16_t pin) {
    if (!DHT_Bus_WaitLevel(port, pin, GPIO_PIN_RESET, DHT_RESPONSE_TIMEOUT_US)) {
        return false;
    }
    if (!DHT_Bus_WaitLevel(port, pin, GPIO_PIN_SET, DHT_RESPONSE_TIMEOUT_US)) {
        return false;
    }

    return DHT_Bus_WaitLevel(port, pin, GPIO_PIN_RESET, DHT_RESPONSE_TIMEOUT_US);
}

/** @brief Clock in 40 bits, each one timed from its rising to its falling edge */
static HAL_StatusTypeDef DHT_Bus_ReadBits(GPIO_TypeDef *port, uint16_t pin, uint8_t *frame) {
    for (uint32_t i = 0U; i < DHT_FRAME_BITS; i++) {
        if (!DHT_Bus_WaitLevel(port, pin, GPIO_PIN_SET, DHT_BIT_LOW_TIMEOUT_US)) {
            return HAL_TIMEOUT;
        }

        uint32_t start = DHT_Timing_Now();

        if (!DHT_Bus_WaitLevel(port, pin, GPIO_PIN_RESET, DHT_BIT_HIGH_TIMEOUT_US)) {
            return HAL_TIMEOUT;
        }

        if (DHT_Timing_ElapsedUs(start) > DHT_BIT_ONE_THRESHOLD_US) {
            frame[i / DHT_BITS_PER_BYTE] |=
                (uint8_t)(1U << ((DHT_BITS_PER_BYTE - 1U) - (i % DHT_BITS_PER_BYTE)));
        }
    }

    return HAL_OK;
}

void DHT_Bus_Release(GPIO_TypeDef *port, uint16_t pin) {
    DHT_Bus_SetOutput(port, pin);
    GPIO_Driver_WritePin(port, pin, GPIO_PIN_SET);
}

HAL_StatusTypeDef DHT_Bus_ReadFrame(GPIO_TypeDef *port, uint16_t pin, uint32_t startPulseMs,
                                    uint8_t *frame) {
    if (port == NULL || frame == NULL) {
        return HAL_ERROR;
    }

    for (uint32_t i = 0U; i < DHT_FRAME_BYTES; i++) {
        frame[i] = 0U;
    }

    DHT_Bus_SetOutput(port, pin);
    GPIO_Driver_WritePin(port, pin, GPIO_PIN_RESET);
    HAL_Delay(startPulseMs);
    GPIO_Driver_WritePin(port, pin, GPIO_PIN_SET);
    DHT_Timing_DelayUs(DHT_START_RELEASE_US);

    DHT_Bus_SetInput(port, pin);

    /* The response and the 40 data bits are decoded from pulse widths of a few
       tens of microseconds, so any interrupt taken here corrupts the frame.
       DWT keeps counting with interrupts masked, and the section is ~5 ms. */
    uint32_t primask = __get_PRIMASK();
    __disable_irq();

    HAL_StatusTypeDef status = HAL_TIMEOUT;
    if (DHT_Bus_AwaitResponse(port, pin)) {
        status = DHT_Bus_ReadBits(port, pin, frame);
    }

    __set_PRIMASK(primask);

    DHT_Bus_Release(port, pin);

    return status;
}
