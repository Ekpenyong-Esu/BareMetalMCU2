/**
 * @file xpt2046_core.h
 * @brief XPT2046 lifetime, calibration and orientation
 */

#ifndef XPT2046_CORE_H
#define XPT2046_CORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xpt2046_types.h"

/**
 * @brief   Initialize the XPT2046 touch controller
 * @note    The SPI bus itself is shared and must already be initialized; this
 *          driver only owns the chip select and PENIRQ pins.
 * @param   hxpt     Handle to initialize
 * @param   cs_port  Chip select port
 * @param   cs_pin   Chip select pin
 * @param   irq_port PENIRQ port
 * @param   irq_pin  PENIRQ pin
 * @param   width    Display width used for coordinate mapping
 * @param   height   Display height used for coordinate mapping
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_Init(XPT2046_Handle_t *hxpt,
                                   GPIO_TypeDef *cs_port, uint16_t cs_pin,
                                   GPIO_TypeDef *irq_port, uint16_t irq_pin,
                                   uint16_t width, uint16_t height);

/**
 * @brief   Release the handle and deselect the controller
 * @param   hxpt Handle to release
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_DeInit(XPT2046_Handle_t *hxpt);

/**
 * @brief   Replace the default raw span with values measured on the panel
 * @details Use XPT2046_ReadRaw() to collect the corner readings; prompting the
 *          user for the corner touches belongs to the application.
 * @param   hxpt      Handle
 * @param   raw_x_min Raw X read at the left edge
 * @param   raw_x_max Raw X read at the right edge
 * @param   raw_y_min Raw Y read at the top edge
 * @param   raw_y_max Raw Y read at the bottom edge
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_SetCalibration(XPT2046_Handle_t *hxpt,
                                             uint16_t raw_x_min, uint16_t raw_x_max,
                                             uint16_t raw_y_min, uint16_t raw_y_max);

/**
 * @brief   Mirror either axis to match how the panel is mounted
 * @param   hxpt   Handle
 * @param   flip_x Mirror the X axis
 * @param   flip_y Mirror the Y axis
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_SetOrientation(XPT2046_Handle_t *hxpt, bool flip_x, bool flip_y);

/**
 * @brief   Check whether the handle has been initialized
 */
bool XPT2046_IsInitialized(const XPT2046_Handle_t *hxpt);

/**
 * @brief   Check whether the panel is currently touched
 * @note    PENIRQ is only meaningful while no conversion is running.
 * @param   hxpt Handle
 * @retval  bool True if touched
 */
bool XPT2046_IsTouched(const XPT2046_Handle_t *hxpt);

/**
 * @brief   Convert a status code to a human readable string
 */
const char *XPT2046_GetStatusString(XPT2046_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* XPT2046_CORE_H */
