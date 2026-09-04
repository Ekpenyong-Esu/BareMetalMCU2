/**
 * @file xpt2046_map.c
 * @brief Raw-to-display coordinate mapping and pressure estimation
 */

#include "xpt2046_map.h"

#define XPT2046_PRESSURE_SCALE 1000U /**< Ratio scale of the pressure estimate */

uint16_t XPT2046_ScaleAxis(uint16_t raw, uint16_t raw_min, uint16_t raw_max, uint16_t span) {
    if (span == 0U || raw_min >= raw_max || raw <= raw_min) {
        return 0U;
    }

    if (raw >= raw_max) {
        return (uint16_t)(span - 1U);
    }

    return (uint16_t)(((uint32_t)(raw - raw_min) * span) / (uint32_t)(raw_max - raw_min));
}

uint16_t XPT2046_CalculatePressure(uint16_t rawZ1, uint16_t rawZ2) {
    uint32_t pressure = 0;

    if (rawZ1 == 0U) {
        return 0U;
    }

    /* Z2 collapses towards zero as the contact hardens, so it is the maximum
       of the scale rather than an absent reading. */
    if (rawZ2 == 0U) {
        return XPT2046_MAX_PRESSURE;
    }

    pressure = ((uint32_t)rawZ1 * XPT2046_PRESSURE_SCALE) / rawZ2;

    return (pressure > XPT2046_MAX_PRESSURE) ? (uint16_t)XPT2046_MAX_PRESSURE : (uint16_t)pressure;
}

XPT2046_StatusTypeDef XPT2046_MapSample(const XPT2046_Config_t *config,
                                        const XPT2046_RawSample_t *sample,
                                        XPT2046_TouchPoint_t *touch) {
    uint16_t posX = 0;
    uint16_t posY = 0;

    if (config == NULL || sample == NULL || touch == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    if (config->width == 0U || config->height == 0U) {
        return XPT2046_INVALID_PARAM;
    }

    posX = XPT2046_ScaleAxis(sample->x, config->raw_x_min, config->raw_x_max, config->width);
    posY = XPT2046_ScaleAxis(sample->y, config->raw_y_min, config->raw_y_max, config->height);

    if (config->flip_x) {
        posX = (uint16_t)(config->width - 1U - posX);
    }
    if (config->flip_y) {
        posY = (uint16_t)(config->height - 1U - posY);
    }

    touch->x = posX;
    touch->y = posY;
    touch->pressure = XPT2046_CalculatePressure(sample->z1, sample->z2);

    return XPT2046_OK;
}
