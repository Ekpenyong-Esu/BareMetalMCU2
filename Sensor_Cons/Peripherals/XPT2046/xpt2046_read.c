/**
 * @file xpt2046_read.c
 * @brief Touch acquisition and press/hold/release tracking
 */

#include "xpt2046_read.h"
#include "xpt2046_core.h"
#include "xpt2046_io.h"
#include "xpt2046_map.h"

/* PENIRQ is pulled up again only once the panel drivers have switched off. */
#define XPT2046_SETTLE_US       20U

XPT2046_StatusTypeDef XPT2046_ReadRaw(XPT2046_Handle_t *hxpt, XPT2046_RawSample_t *sample)
{
    XPT2046_StatusTypeDef status;

    if (hxpt == NULL || sample == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    if (!hxpt->initialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    if (!XPT2046_IO_PenDown(&hxpt->config)) {
        return XPT2046_NO_TOUCH;
    }

    status = XPT2046_IO_ReadSample(&hxpt->config, sample);
    if (status != XPT2046_OK) {
        return status;
    }

    /* A pen lifted during the four conversions leaves the position channels
       floating, so the sample has to be discarded rather than reported. */
    XPT2046_IO_DelayUs(XPT2046_SETTLE_US);
    if (sample->z1 == 0U || !XPT2046_IO_PenDown(&hxpt->config)) {
        return XPT2046_NO_TOUCH;
    }

    return XPT2046_OK;
}

XPT2046_StatusTypeDef XPT2046_ReadTouch(XPT2046_Handle_t *hxpt, XPT2046_TouchPoint_t *touch)
{
    XPT2046_RawSample_t sample;
    XPT2046_StatusTypeDef status;

    if (hxpt == NULL || touch == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    status = XPT2046_ReadRaw(hxpt, &sample);
    if (status != XPT2046_OK) {
        touch->x = 0U;
        touch->y = 0U;
        touch->pressure = 0U;
        touch->state = XPT2046_STATE_RELEASED;
        return status;
    }

    status = XPT2046_MapSample(&hxpt->config, &sample, touch);
    if (status != XPT2046_OK) {
        return status;
    }

    touch->state = XPT2046_STATE_PRESSED;

    return XPT2046_OK;
}

XPT2046_StatusTypeDef XPT2046_Update(XPT2046_Handle_t *hxpt)
{
    XPT2046_TouchPoint_t sampled;
    XPT2046_StatusTypeDef status;

    if (hxpt == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    if (!hxpt->initialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    status = XPT2046_ReadTouch(hxpt, &sampled);

    if (status == XPT2046_OK) {
        bool wasDown = (hxpt->touch.state != XPT2046_STATE_RELEASED);

        hxpt->touch = sampled;
        hxpt->touch.state = wasDown ? XPT2046_STATE_HELD : XPT2046_STATE_PRESSED;
        return XPT2046_OK;
    }

    if (status == XPT2046_NO_TOUCH) {
        hxpt->touch.state = XPT2046_STATE_RELEASED;
        hxpt->touch.pressure = 0U;
        return XPT2046_OK;
    }

    /* A bus failure says nothing about the contact, so the tracked state is
       left alone for the caller to retry. */
    return status;
}

XPT2046_StatusTypeDef XPT2046_GetTouch(const XPT2046_Handle_t *hxpt, XPT2046_TouchPoint_t *touch)
{
    if (hxpt == NULL || touch == NULL) {
        return XPT2046_INVALID_PARAM;
    }

    if (!hxpt->initialized) {
        return XPT2046_NOT_INITIALIZED;
    }

    *touch = hxpt->touch;

    return XPT2046_OK;
}
