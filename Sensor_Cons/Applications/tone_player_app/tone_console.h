/**
 * @file tone_console.h
 * @brief Says over the serial port what the tone player is doing
 *
 * Reporting only. It formats nothing about the board and decides nothing
 * about the music: callers hand it the words, it puts them on the wire.
 * Delete this module and the buzzer still plays.
 */

#ifndef TONE_CONSOLE_H
#define TONE_CONSOLE_H

#include "melody.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Open the serial port used for reporting
 * @note   Never fails loudly. If the port will not open every later call goes
 *         quiet, because a missing serial cable is no reason to stop playing.
 */
void ToneConsole_Init(void);

/**
 * @brief  Announce that the player has started
 * @param  outputDescription Which buzzer is wired where; the caller knows the
 *                           board, this module does not
 */
void ToneConsole_ReportReady(const char *outputDescription);

/**
 * @brief  Announce the tune about to be played
 * @param  melody Tune whose title to print
 */
void ToneConsole_ReportMelody(const Melody_t *melody);

/**
 * @brief  Report that the player could not start
 * @param  reason Short explanation to print
 */
void ToneConsole_ReportError(const char *reason);

#ifdef __cplusplus
}
#endif

#endif /* TONE_CONSOLE_H */
