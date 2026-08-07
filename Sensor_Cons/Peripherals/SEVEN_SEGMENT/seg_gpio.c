/**
 * @file    seg_gpio.c
 * @brief   Direct-GPIO seven-segment backend implementation
 * @details Drives one digit at a time; the caller is responsible for calling
 *          Seg_Update fast enough to keep the display flicker free.
 */

/* Includes ------------------------------------------------------------------*/
#include "seg_gpio.h"

#include "seg_font.h"
#include "gpio.h"

/* Private functions ---------------------------------------------------------*/

static void Seg_GpioConfigurePin(const SegGpioPin_t* pin)
{
    if (pin->port == NULL) {
        return;
    }

    GPIO_InitTypeDef init = {0};
    init.Pin = pin->pin;
    init.Mode = GPIO_MODE_OUTPUT_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_Driver_Pin_Init(pin->port, &init);
}

static void Seg_GpioWriteSegments(SegDisplayHandle_t* handle, uint8_t pattern)
{
    const SegGpioConfig_t* cfg = &handle->config.config.gpio;
    bool invert = (cfg->polarity == SEG_COMMON_ANODE);

    for (uint8_t i = 0; i < SEG_COUNT; i++) {
        if (cfg->segments[i].port == NULL) {
            continue;
        }
        bool on = ((pattern >> i) & 0x01U) != 0U;
        if (invert) {
            on = !on;
        }
        HAL_GPIO_WritePin(cfg->segments[i].port, cfg->segments[i].pin,
                          on ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

static void Seg_GpioDeselectAll(SegDisplayHandle_t* handle)
{
    const SegGpioConfig_t* cfg = &handle->config.config.gpio;
    if (cfg->digits == NULL) {
        return;
    }

    GPIO_PinState inactive = cfg->digitActiveHigh ? GPIO_PIN_RESET : GPIO_PIN_SET;
    for (uint8_t i = 0; i < cfg->digitCount; i++) {
        if (cfg->digits[i].port != NULL) {
            HAL_GPIO_WritePin(cfg->digits[i].port, cfg->digits[i].pin, inactive);
        }
    }
}

static void Seg_GpioSelectDigit(SegDisplayHandle_t* handle, uint8_t digit)
{
    const SegGpioConfig_t* cfg = &handle->config.config.gpio;
    if (cfg->digits == NULL || digit >= cfg->digitCount) {
        return;
    }

    GPIO_PinState active = cfg->digitActiveHigh ? GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(cfg->digits[digit].port, cfg->digits[digit].pin, active);
}

/* Backend operations --------------------------------------------------------*/

static SegStatus_t Seg_GpioInit(SegDisplayHandle_t* handle)
{
    const SegGpioConfig_t* cfg = &handle->config.config.gpio;

    if (cfg->digits == NULL) {
        return SEG_INVALID_PARAM;
    }

    /* The GPIO driver enables the port clock for each pin it configures */
    for (uint8_t i = 0; i < SEG_COUNT; i++) {
        Seg_GpioConfigurePin(&cfg->segments[i]);
    }
    for (uint8_t i = 0; i < cfg->digitCount; i++) {
        Seg_GpioConfigurePin(&cfg->digits[i]);
    }

    Seg_GpioDeselectAll(handle);
    Seg_GpioWriteSegments(handle, SEG_PATTERN_BLANK);

    return SEG_OK;
}

static void Seg_GpioEnable(SegDisplayHandle_t* handle)
{
    (void)handle;   /* Output resumes on the next multiplex step */
}

static void Seg_GpioDisable(SegDisplayHandle_t* handle)
{
    Seg_GpioDeselectAll(handle);
}

static void Seg_GpioCommit(SegDisplayHandle_t* handle)
{
    (void)handle;   /* Buffer is sampled by the multiplex step, nothing to push */
}

static void Seg_GpioMultiplexStep(SegDisplayHandle_t* handle)
{
    Seg_GpioDeselectAll(handle);
    Seg_GpioWriteSegments(handle, handle->displayBuffer[handle->currentDigit]);
    Seg_GpioSelectDigit(handle, handle->currentDigit);

    handle->currentDigit++;
    if (handle->currentDigit >= handle->digitCount) {
        handle->currentDigit = 0;
    }
}

const SegDriverOps_t SegGpioOps = {
    .init          = Seg_GpioInit,
    .enable        = Seg_GpioEnable,
    .disable       = Seg_GpioDisable,
    .commit        = Seg_GpioCommit,
    .multiplexStep = Seg_GpioMultiplexStep,
};
