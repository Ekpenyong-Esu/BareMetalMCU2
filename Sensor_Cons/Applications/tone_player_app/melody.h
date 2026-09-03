/**
 * @file melody.h
 * @brief Tunes: sequences of notes, read at a tempo
 *
 * Builds on note.h. Where a note is one pitch, a melody is the order they
 * come in and the speed they are read at. Still no hardware anywhere.
 */

#ifndef MELODY_H
#define MELODY_H

#include "note.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** One entry of a tune: which pitch, in which octave, for how long. */
typedef struct {
    NoteName_t   name;
    uint8_t      octave;
    NoteLength_t length;
} MelodyStep_t;

/** A tune: the steps, and the speed they are read at. */
typedef struct {
    const char         *title;
    const MelodyStep_t *steps;
    uint16_t            stepCount;
    uint16_t            tempoBpm; /**< Quarter notes per minute */
} Melody_t;

/**
 * @brief  How long one step lasts at this tune's tempo
 * @param  melody Tune the step belongs to
 * @param  length Length of the step
 * @retval Duration in milliseconds, or 0 if the tempo is unusable
 */
uint32_t Melody_StepDurationMs(const Melody_t *melody, NoteLength_t length);

/** Eight notes up and back down: the quickest way to hear if wiring works. */
extern const Melody_t MELODY_C_MAJOR_SCALE;

/** Traditional, and short enough to recognise a wrong note immediately. */
extern const Melody_t MELODY_TWINKLE;

/** Beethoven's theme: uses a dotted note, so it also exercises the timing. */
extern const Melody_t MELODY_ODE_TO_JOY;

#ifdef __cplusplus
}
#endif

#endif /* MELODY_H */
