/**
 * @file servo_console.h
 * @brief Says over the serial port what the servo sweep is doing
 *
 * Reporting only. It decides nothing about the board and nothing about the
 * motion: the application tells it which port to open and hands it the
 * words, it puts them on the wire. Delete this module and the servo still
 * sweeps.
 */

#ifndef SERVO_CONSOLE_H
#define SERVO_CONSOLE_H

#include "stm32f4xx.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Open the serial port used for reporting at the project default
 *        baud rate. Later calls go quiet if it fails.
 * @param instance  Which USART to talk on, e.g. USART1
 */
void ServoConsole_Init(USART_TypeDef *instance);

/** Announce that the sweep has started and which output drives the servo. */
void ServoConsole_ReportReady(const char *outputDescription);

/** Name the servo whose angles follow, so two servos are told apart. */
void ServoConsole_ReportTurn(const char *servoName);

/** Report one settled angle, e.g. "angle: 90 deg". */
void ServoConsole_ReportAngle(uint16_t angleDeg);

/** Report that the sweep could not start or stopped early. */
void ServoConsole_ReportError(const char *reason);

#ifdef __cplusplus
}
#endif

#endif /* SERVO_CONSOLE_H */
