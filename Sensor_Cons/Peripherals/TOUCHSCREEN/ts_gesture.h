/**
 * @file ts_gesture.h
 * @brief Gesture recognition from cached touch data
 */

#ifndef TS_GESTURE_H
#define TS_GESTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ts_types.h"

/**
 * @brief Classify the movement between the previous and current touch data
 * @param hts Touchscreen handle
 * @return TS_GestureTypeDef Detected gesture, TS_GESTURE_NONE when undecided
 */
TS_GestureTypeDef TS_AnalyzeGesture(const TS_HandleTypeDef *hts);

#ifdef __cplusplus
}
#endif

#endif /* TS_GESTURE_H */
