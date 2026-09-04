/**
 * @file xpt2046.c
 * @brief XPT2046 lifetime, calibration and orientation
 */

#include "xpt2046_core.h"
#include "xpt2046_io.h"
#include <string.h>

XPT2046_StatusTypeDef XPT2046_Init(XPT2046_Handle_t *hxpt, SPI_Bus_t *bus, GPIO_TypeDef *cs_port,
                                   uint16_t cs_pin, GPIO_TypeDef *irq_port, uint16_t irq_pin,
                                   uint16_t width, uint16_t height) {
    XPT2046_StatusTypeDef status = XPT2046_OK;

    if (hxpt == NULL || bus == NULL || cs_port == NULL || irq_port == NULL || cs_pin == 0U ||
        irq_pin == 0U || width == 0U || height == 0U) {
        return XPT2046_INVALID_PARAM;
    }

    memset(hxpt, 0, sizeof(*hxpt));

    hxpt->config.bus = bus;
    hxpt->config.cs_port = cs_port;
    hxpt->config.cs_pin = cs_pin;
    hxpt->config.irq_port = irq_port;
    hxpt->config.irq_pin = irq_pin;
    hxpt->config.width = width;
    hxpt->config.height = height;
    hxpt->config.raw_x_min = XPT2046_RAW_X_MIN_DEFAULT;
    hxpt->config.raw_x_max = XPT2046_RAW_X_MAX_DEFAULT;
    hxpt->config.raw_y_min = XPT2046_RAW_Y_MIN_DEFAULT;
    hxpt->config.raw_y_max = XPT2046_RAW_Y_MAX_DEFAULT;

    status = XPT2046_IO_ConfigurePins(hxpt);
    if (status != XPT2046_OK) {
        return status;
    }

    hxpt->touch.state = XPT2046_STATE_RELEASED;
    hxpt->initialized = true;

    return XPT2046_OK;
}

XPT2046_StatusTypeDef XPT2046_DeInit(XPT2046_Handle_t *hxpt) {
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    if (!hxpt->initialized) {
        return XPT2046_OK;
    }

    HAL_GPIO_WritePin(hxpt->config.cs_port, hxpt->config.cs_pin, GPIO_PIN_SET);
    memset(hxpt, 0, sizeof(*hxpt));

    return XPT2046_OK;
}

XPT2046_StatusTypeDef XPT2046_SetCalibration(XPT2046_Handle_t *hxpt, uint16_t raw_x_min,
                                             uint16_t raw_x_max, uint16_t raw_y_min,
                                             uint16_t raw_y_max) {
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    if (!hxpt->initialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    /* An empty span would collapse every touch onto one display edge. */
    if (raw_x_min >= raw_x_max || raw_y_min >= raw_y_max) {
        return XPT2046_INVALID_PARAM;
    }

    if (raw_x_max > XPT2046_ADC_MAX || raw_y_max > XPT2046_ADC_MAX) {
        return XPT2046_INVALID_PARAM;
    }

    hxpt->config.raw_x_min = raw_x_min;
    hxpt->config.raw_x_max = raw_x_max;
    hxpt->config.raw_y_min = raw_y_min;
    hxpt->config.raw_y_max = raw_y_max;

    return XPT2046_OK;
}

XPT2046_StatusTypeDef XPT2046_SetOrientation(XPT2046_Handle_t *hxpt, bool flip_x, bool flip_y) {
    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    if (!hxpt->initialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    hxpt->config.flip_x = flip_x;
    hxpt->config.flip_y = flip_y;

    return XPT2046_OK;
}

bool XPT2046_IsInitialized(const XPT2046_Handle_t *hxpt) {
    return (hxpt != NULL) && hxpt->initialized;
}

bool XPT2046_IsTouched(const XPT2046_Handle_t *hxpt) {
    if (!XPT2046_IsInitialized(hxpt)) {
        return false;
    }

    return XPT2046_IO_PenDown(&hxpt->config);
}

const char *XPT2046_GetStatusString(XPT2046_StatusTypeDef status) {
    switch (status) {
        case XPT2046_OK:
            return "XPT2046_OK";
        case XPT2046_ERROR:
            return "XPT2046_ERROR";
        case XPT2046_BUSY:
            return "XPT2046_BUSY";
        case XPT2046_TIMEOUT:
            return "XPT2046_TIMEOUT";
        case XPT2046_INVALID_PARAM:
            return "XPT2046_INVALID_PARAM";
        case XPT2046_NOT_INITIALIZED:
            return "XPT2046_NOT_INITIALIZED";
        case XPT2046_NO_TOUCH:
            return "XPT2046_NO_TOUCH";
        default:
            return "UNKNOWN_STATUS";
    }
}
