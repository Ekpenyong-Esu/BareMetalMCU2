/**
 * @file    seg_core.c
 * @brief   Seven-segment lifecycle implementation
 */

/* Includes ------------------------------------------------------------------*/
#include "seg_core.h"

#include "seg_font.h"
#include "seg_gpio.h"
#include "seg_ht1621.h"

#include <string.h>

/* Private functions ---------------------------------------------------------*/

/** @brief The only place the driver type is inspected */
static const SegDriverOps_t *Seg_OpsFor(SegDriverType_t type) {
    switch (type) {
        case SEG_DRIVER_GPIO:
            return &SegGpioOps;
        case SEG_DRIVER_HT1621:
            return &SegHt1621Ops;
        default:
            return NULL;
    }
}

static uint8_t Seg_ConfiguredDigitCount(const SegDisplayConfig_t *config) {
    return (config->driverType == SEG_DRIVER_GPIO) ? config->config.gpio.digitCount
                                                   : config->config.ht1621.digitCount;
}

/* Exported functions --------------------------------------------------------*/

SegStatus_t Seg_Init(SegDisplayHandle_t *handle, const SegDisplayConfig_t *config) {
    if (handle == NULL || config == NULL) {
        return SEG_INVALID_PARAM;
    }

    const SegDriverOps_t *ops = Seg_OpsFor(config->driverType);
    if (ops == NULL) {
        return SEG_INVALID_PARAM;
    }

    uint8_t digitCount = Seg_ConfiguredDigitCount(config);
    if (digitCount == 0 || digitCount > SEG_MAX_DIGITS) {
        return SEG_INVALID_PARAM;
    }

    memset(handle, 0, sizeof(*handle));
    handle->config = *config;
    handle->ops = ops;
    handle->digitCount = digitCount;

    SegStatus_t status = ops->init(handle);
    if (status != SEG_OK) {
        handle->ops = NULL;
        return status;
    }

    handle->enabled = true;
    handle->initialized = true;

    return SEG_OK;
}

SegStatus_t Seg_DeInit(SegDisplayHandle_t *handle) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    Seg_Disable(handle);
    handle->initialized = false;

    return SEG_OK;
}

SegStatus_t Seg_Enable(SegDisplayHandle_t *handle) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    handle->enabled = true;
    handle->ops->enable(handle);

    return SEG_OK;
}

SegStatus_t Seg_Disable(SegDisplayHandle_t *handle) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    handle->enabled = false;
    handle->ops->disable(handle);

    return SEG_OK;
}

SegStatus_t Seg_Clear(SegDisplayHandle_t *handle) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    memset(handle->displayBuffer, SEG_PATTERN_BLANK, handle->digitCount);

    return Seg_Refresh(handle);
}

SegStatus_t Seg_Update(SegDisplayHandle_t *handle) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    if (handle->enabled && handle->ops->multiplexStep != NULL) {
        handle->ops->multiplexStep(handle);
    }

    return SEG_OK;
}

SegStatus_t Seg_Refresh(SegDisplayHandle_t *handle) {
    SegStatus_t status = Seg_CheckReady(handle);
    if (status != SEG_OK) {
        return status;
    }

    handle->ops->commit(handle);

    return SEG_OK;
}

uint8_t Seg_GetDigitCount(const SegDisplayHandle_t *handle) {
    return (Seg_CheckReady(handle) == SEG_OK) ? handle->digitCount : 0U;
}
