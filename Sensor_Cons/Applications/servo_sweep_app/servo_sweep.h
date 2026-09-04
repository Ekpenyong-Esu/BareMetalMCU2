/**
 * @file servo_sweep.h
 * @brief Sweep pattern: which angles, in which order, how fast
 *
 * Knows nothing about pins, timers, or UART; it touches the HAL only for
 * HAL_Delay. It moves any initialised servo through min -> max -> min and
 * optionally reports each step through a caller-supplied callback.
 */

#ifndef SERVO_SWEEP_H
#define SERVO_SWEEP_H

#include "servo.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Sweep shape; all fields in degrees or milliseconds. */
typedef struct {
    uint16_t minAngle;    /**< Bottom of the sweep */
    uint16_t maxAngle;    /**< Top of the sweep */
    uint16_t stepDeg;     /**< Angle increment per step */
    uint16_t stepDelayMs; /**< Settle time after each step */
} ServoSweep_Config_t;

/** Called after each settled step; NULL skips reporting. */
typedef void (*ServoSweep_Progress_t)(uint16_t angleDeg);

/**
 * @brief Full travel of the given servo in 5 deg steps with 30 ms settle time
 * @param hservo  Servo whose configured limits bound the sweep; NULL falls
 *                back to the driver's 0..180 deg range
 */
ServoSweep_Config_t ServoSweep_GetDefaultConfig(const SERVO_Handle_t *hservo);

/**
 * @brief Move to min, sweep up to max, then back down to min
 *
 * Every angle is commanded and reported exactly once per pass, including
 * the two turnaround points.
 *
 * @retval SERVO_OK when the full pass completed
 */
SERVO_StatusTypeDef ServoSweep_RunOnce(SERVO_Handle_t *hservo, const ServoSweep_Config_t *config,
                                       ServoSweep_Progress_t progress);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_SWEEP_H */
