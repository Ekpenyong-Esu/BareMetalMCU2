/**
 * @file tone_player_app.h
 * @brief Application 6, "Tone Player": beeps and melodies on a buzzer
 */

#ifndef TONE_PLAYER_APP_H
#define TONE_PLAYER_APP_H

#include "buzzer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Wire up a buzzer and play the built-in tunes on a loop
 * @param  mode BUZZER_MODE_PASSIVE_PWM for a passive buzzer on PE5 (P1-14),
 *              BUZZER_MODE_ACTIVE_GPIO for an active one on PE6 (P1-11)
 * @note   Only a passive buzzer can carry the melody; an active one plays the
 *         rhythm at whatever single pitch its oscillator was built for.
 */
void TonePlayerApp_Run(BuzzerMode_t mode);

#ifdef __cplusplus
}
#endif

#endif /* TONE_PLAYER_APP_H */
