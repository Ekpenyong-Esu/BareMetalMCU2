/**
 * @file    seg_ht1621.h
 * @brief   HT1621 LCD-controller seven-segment backend
 */

#ifndef SEG_HT1621_H
#define SEG_HT1621_H

#ifdef __cplusplus
extern "C" {
#endif

#include "seg_types.h"

/** @brief Operations for a display driven through an HT1621 */
extern const SegDriverOps_t SegHt1621Ops;

#ifdef __cplusplus
}
#endif

#endif /* SEG_HT1621_H */
