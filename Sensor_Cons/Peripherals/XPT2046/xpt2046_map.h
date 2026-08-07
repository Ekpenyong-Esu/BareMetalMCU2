/**
 * @file xpt2046_map.h
 * @brief Raw-to-display coordinate mapping and pressure estimation
 */

#ifndef XPT2046_MAP_H
#define XPT2046_MAP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xpt2046_types.h"

/**
 * @brief   Scale one raw axis reading onto 0..span-1
 * @param   raw     Raw ADC reading
 * @param   raw_min Raw value at the start of the axis
 * @param   raw_max Raw value at the end of the axis
 * @param   span    Display size along the axis
 * @retval  uint16_t Display coordinate, clamped to the span
 */
uint16_t XPT2046_ScaleAxis(uint16_t raw, uint16_t raw_min, uint16_t raw_max, uint16_t span);

/**
 * @brief   Estimate touch pressure from the Z channels
 * @details Reports the Z1/Z2 ratio, which rises with contact force. It is not a
 *          calibrated resistance: that needs the panel plate resistance, which
 *          this driver does not know.
 * @param   z1 Raw Z1 reading
 * @param   z2 Raw Z2 reading
 * @retval  uint16_t Pressure, 0..XPT2046_MAX_PRESSURE
 */
uint16_t XPT2046_CalculatePressure(uint16_t z1, uint16_t z2);

/**
 * @brief   Convert a raw sample into display coordinates and pressure
 * @param   config Calibration and orientation to apply
 * @param   sample Raw channel readings
 * @param   touch  Receives the mapped point, state left untouched
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_MapSample(const XPT2046_Config_t *config,
                                        const XPT2046_RawSample_t *sample,
                                        XPT2046_TouchPoint_t *touch);

#ifdef __cplusplus
}
#endif

#endif /* XPT2046_MAP_H */
