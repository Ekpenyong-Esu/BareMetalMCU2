/**
 * @file note.h
 * @brief What a single note is: its pitch and its length
 *
 * The pitch system on its own. It answers one question, "what frequency is
 * this note?", and knows nothing about tunes, tempo or hardware.
 */

#ifndef NOTE_H
#define NOTE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** The twelve semitones of an octave, plus silence. */
typedef enum {
    NOTE_C = 0,
    NOTE_CS,
    NOTE_D,
    NOTE_DS,
    NOTE_E,
    NOTE_F,
    NOTE_FS,
    NOTE_G,
    NOTE_GS,
    NOTE_A,
    NOTE_AS,
    NOTE_B,
    NOTE_REST
} NoteName_t;

/** How long a note lasts, counted in sixteenths of a whole note. */
typedef enum {
    NOTE_SIXTEENTH      = 1,
    NOTE_EIGHTH         = 2,
    NOTE_DOTTED_EIGHTH  = 3,
    NOTE_QUARTER        = 4,
    NOTE_DOTTED_QUARTER = 6,
    NOTE_HALF           = 8,
    NOTE_DOTTED_HALF    = 12,
    NOTE_WHOLE          = 16
} NoteLength_t;

/** Sixteenths in one beat, a beat being a quarter note. */
#define NOTE_SIXTEENTHS_PER_BEAT 4U

/**
 * @brief  Pitch of a note in hertz
 * @param  name   Semitone, or NOTE_REST for silence
 * @param  octave 0 to 8, in scientific pitch notation, so octave 4 holds A440
 * @retval Frequency in Hz, or 0 for a rest or an octave outside the range
 */
uint32_t Note_FrequencyHz(NoteName_t name, uint8_t octave);

#ifdef __cplusplus
}
#endif

#endif /* NOTE_H */
