/**
 * @file note.c
 * @brief What a single note is: its pitch and its length
 */

#include "note.h"

/** Highest octave the table covers; every lower one is derived from it. */
#define NOTE_TABLE_OCTAVE 8U

/* Octave 8 in hertz. Only one octave needs storing because doubling the
   frequency raises the pitch by exactly one octave, so the rest is a shift.
   Sizing the table by NOTE_REST keeps it in step with the enum above it. */
static const uint16_t s_octave8Hz[NOTE_REST] = {
    4186U, 4435U, 4699U, 4978U, 5274U, 5588U,
    5920U, 6272U, 6645U, 7040U, 7459U, 7902U
};

uint32_t Note_FrequencyHz(NoteName_t name, uint8_t octave)
{
    if (name >= NOTE_REST || octave > NOTE_TABLE_OCTAVE) {
        return 0U;
    }

    return (uint32_t)s_octave8Hz[name] >> (NOTE_TABLE_OCTAVE - octave);
}
