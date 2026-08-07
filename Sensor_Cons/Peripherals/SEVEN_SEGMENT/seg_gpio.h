/**
 * @file    seg_gpio.h
 * @brief   Direct-GPIO (multiplexed) seven-segment backend
 */

#ifndef SEG_GPIO_H
#define SEG_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "seg_types.h"

/** @brief Operations for a display wired straight to GPIO pins */
extern const SegDriverOps_t SegGpioOps;

#ifdef __cplusplus
}
#endif

#endif /* SEG_GPIO_H */
