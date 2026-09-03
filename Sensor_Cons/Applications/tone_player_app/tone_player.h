/**
 * @file tone_player.h
 * @brief Plays notes and melodies through a passive buzzer
 *
 * The layer between the music and the hardware: it knows how to turn a pitch
 * into a tone and a tempo into a delay, but not which pin or timer the buzzer
 * hangs off. The caller supplies those once, at init.
 */

#ifndef TONE_PLAYER_H
#define TONE_PLAYER_H

#include "buzzer.h"
#include "melody.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief  Prepare a timer channel and bind a passive buzzer to it
 * @param  buzzer   Buzzer to fill in; owned by the caller
 * @param  htim     Timer handle, kept for as long as the buzzer is used
 * @param  instance Timer peripheral driving the buzzer pin
 * @param  channel  TIM_CHANNEL_1 to TIM_CHANNEL_4, matching that pin
 * @retval HAL_OK on success
 * @note   The pin itself must already be in alternate-function mode: which pin
 *         it is, is a board question and belongs to the caller.
 * @note   There is no matching InitActive, because an active buzzer needs no
 *         timer at all: Buzzer_InitActive() on its own is enough.
 */
HAL_StatusTypeDef TonePlayer_InitPassive(Buzzer_t *buzzer,
                                         TIM_HandleTypeDef *htim,
                                         TIM_TypeDef *instance,
                                         uint32_t channel);

/**
 * @brief  Sound one frequency for a while, then go quiet
 * @param  buzzer      Initialised buzzer, either kind
 * @param  frequencyHz Pitch; frequencies the timer cannot reach are skipped
 * @param  durationMs  How long to hold it
 * @note   An active buzzer ignores the frequency and sounds its own pitch.
 */
void TonePlayer_Beep(Buzzer_t *buzzer, uint32_t frequencyHz, uint32_t durationMs);

/**
 * @brief  Sound one note for a while
 * @param  buzzer     Initialised buzzer, either kind
 * @param  name       Semitone, or NOTE_REST to stay silent for the duration
 * @param  octave     0 to 8
 * @param  durationMs How long to hold it
 */
void TonePlayer_PlayNote(Buzzer_t *buzzer,
                         NoteName_t name,
                         uint8_t octave,
                         uint32_t durationMs);

/**
 * @brief  Play a whole tune, blocking until the last note ends
 * @param  buzzer Initialised buzzer, either kind
 * @param  melody Tune to play
 * @note   An active buzzer reproduces the rhythm only: the oscillator inside
 *         it has one pitch and the pin can only gate it.
 */
void TonePlayer_PlayMelody(Buzzer_t *buzzer, const Melody_t *melody);

#ifdef __cplusplus
}
#endif

#endif /* TONE_PLAYER_H */
