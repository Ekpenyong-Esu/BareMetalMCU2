/**
 * @file ts_gesture.c
 * @brief Gesture recognition from cached touch data
 */

#include "ts_gesture.h"

#define TS_GESTURE_THRESHOLD 20 /*!< Minimum movement in pixels for a swipe */
#define TS_LONG_PRESS_TIME 1000 /*!< Minimum press duration for a long press (ms) */

/**
 * @brief Absolute value of a signed delta
 * @param value Signed delta
 * @return int32_t Magnitude
 */
static int32_t TS_Abs(int32_t value) {
    return (value < 0) ? -value : value;
}

TS_GestureTypeDef TS_AnalyzeGesture(const TS_HandleTypeDef *hts) {
    if (hts->TouchData.TouchCount == 0 && hts->PrevTouchData.TouchCount > 0) {
        uint32_t duration = HAL_GetTick() - hts->PrevTouchData.Points[0].Timestamp;
        return (duration > TS_LONG_PRESS_TIME) ? TS_GESTURE_LONG_PRESS : TS_GESTURE_TAP;
    }

    if (hts->TouchData.TouchCount > 0 && hts->PrevTouchData.TouchCount > 0) {
        int32_t deltaX =
            (int32_t)hts->TouchData.Points[0].X - (int32_t)hts->PrevTouchData.Points[0].X;
        int32_t deltaY =
            (int32_t)hts->TouchData.Points[0].Y - (int32_t)hts->PrevTouchData.Points[0].Y;

        /* Compare squared distances so no square root is needed. */
        if (((deltaX * deltaX) + (deltaY * deltaY)) >
            (TS_GESTURE_THRESHOLD * TS_GESTURE_THRESHOLD)) {
            if (TS_Abs(deltaX) > TS_Abs(deltaY)) {
                return (deltaX > 0) ? TS_GESTURE_SWIPE_RIGHT : TS_GESTURE_SWIPE_LEFT;
            }
            return (deltaY > 0) ? TS_GESTURE_SWIPE_DOWN : TS_GESTURE_SWIPE_UP;
        }
    }

    return TS_GESTURE_NONE;
}
