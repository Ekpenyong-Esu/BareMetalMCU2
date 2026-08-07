/**
 * @file xpt2046_io.h
 * @brief Internal SPI/GPIO transport for the XPT2046 controller
 * @note  Not part of the public xpt2046.h aggregator.
 */

#ifndef XPT2046_IO_H
#define XPT2046_IO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "xpt2046_types.h"

/**
 * @brief   Configure the chip select output and the PENIRQ input
 * @param   config Pin configuration
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_IO_ConfigurePins(const XPT2046_Config_t *config);

/**
 * @brief   Read the PENIRQ line
 * @retval  bool True while the pen is down (line low)
 */
bool XPT2046_IO_PenDown(const XPT2046_Config_t *config);

/**
 * @brief   Acquire X, Y, Z1 and Z2 in one chip select sequence
 * @param   config Pin configuration
 * @param   sample Receives the raw channel readings
 * @retval  XPT2046_StatusTypeDef Operation status
 */
XPT2046_StatusTypeDef XPT2046_IO_ReadSample(const XPT2046_Config_t *config,
                                            XPT2046_RawSample_t *sample);

/**
 * @brief   Busy-wait for approximately the given number of microseconds
 */
void XPT2046_IO_DelayUs(uint32_t microseconds);

#ifdef __cplusplus
}
#endif

#endif /* XPT2046_IO_H */
