/**
 * @file xpt2046_read.h
 * @brief Touch acquisition and press/hold/release tracking
 */

#ifndef XPT2046_READ_H
#define XPT2046_READ_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xpt2046_types.h"

/**
 * @brief   Acquire one unmapped sample of all four channels
 * @details Intended for calibration; no mapping or orientation is applied.
 * @param   hxpt   Handle
 * @param   sample Receives the raw readings
 * @retval  XPT2046_StatusTypeDef Operation status, XPT2046_NO_TOUCH if released
 */
XPT2046_StatusTypeDef XPT2046_ReadRaw(XPT2046_Handle_t *hxpt, XPT2046_RawSample_t *sample);

/**
 * @brief   Read one mapped touch point
 * @details On XPT2046_NO_TOUCH the coordinates are zeroed rather than left stale.
 * @param   hxpt  Handle
 * @param   touch Receives the mapped point
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_ReadTouch(XPT2046_Handle_t *hxpt, XPT2046_TouchPoint_t *touch);

/**
 * @brief   Sample the panel and advance the released/pressed/held state
 * @param   hxpt Handle
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Update(XPT2046_Handle_t *hxpt);

/**
 * @brief   Read back the state maintained by XPT2046_Update()
 * @param   hxpt  Handle
 * @param   touch Receives a copy of the tracked touch point
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_GetTouch(const XPT2046_Handle_t *hxpt, XPT2046_TouchPoint_t *touch);

#ifdef __cplusplus
}
#endif

#endif /* XPT2046_READ_H */
