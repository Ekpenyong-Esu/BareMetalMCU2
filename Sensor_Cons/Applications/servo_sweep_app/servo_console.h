/**
 * @file servo_console.h
 * @brief Says over the serial port what the servo sweep is doing
 *
 * Reporting only. It formats nothing about the board and decides nothing
 * about the motion: callers hand it the words, it puts them on the wire.
 * Delete this module and the servo still sweeps.
 */

#ifndef SERVO_CONSOLE_H
#define SERVO_CONSOLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Open the serial port used for reporting. Later calls go quiet if it fails. */
void ServoConsole_Init(void);

/** Announce that the sweep has started and which output drives the servo. */
void ServoConsole_ReportReady(const char *outputDescription);

/** Report one settled angle, e.g. "angle: 90 deg". */
void ServoConsole_ReportAngle(uint16_t angleDeg);

/** Report that the sweep could not start or stopped early. */
void ServoConsole_ReportError(const char *reason);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CONSOLE_H */
