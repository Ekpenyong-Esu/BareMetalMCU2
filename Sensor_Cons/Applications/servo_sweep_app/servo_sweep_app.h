/**
 * @file servo_sweep_app.h
 * @brief Application 7, "Servo Sweep": drive a hobby servo through its range
 */

#ifndef SERVO_SWEEP_APP_H
#define SERVO_SWEEP_APP_H

#ifdef __cplusplus
extern "C" {
#endif

/** Wire up the servo and sweep it on a loop, reporting each angle on UART. */
void ServoSweepApp_Run(void);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_SWEEP_APP_H */
