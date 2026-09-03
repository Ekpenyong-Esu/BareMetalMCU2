/**
 * @file melody.c
 * @brief Tunes: sequences of notes, read at a tempo
 */

#include "melody.h"

#include <stddef.h>

#define MELODY_STEP_COUNT(steps) ((uint16_t)(sizeof(steps) / sizeof((steps)[0])))

#define MELODY_MS_PER_MINUTE 60000U

/* --------------------------------------------------------------------------
 * Tunes
 * -------------------------------------------------------------------------- */

static const MelodyStep_t s_cMajorScaleSteps[] = {
    { NOTE_C, 4, NOTE_EIGHTH }, { NOTE_D, 4, NOTE_EIGHTH },
    { NOTE_E, 4, NOTE_EIGHTH }, { NOTE_F, 4, NOTE_EIGHTH },
    { NOTE_G, 4, NOTE_EIGHTH }, { NOTE_A, 4, NOTE_EIGHTH },
    { NOTE_B, 4, NOTE_EIGHTH }, { NOTE_C, 5, NOTE_QUARTER },
    { NOTE_B, 4, NOTE_EIGHTH }, { NOTE_A, 4, NOTE_EIGHTH },
    { NOTE_G, 4, NOTE_EIGHTH }, { NOTE_F, 4, NOTE_EIGHTH },
    { NOTE_E, 4, NOTE_EIGHTH }, { NOTE_D, 4, NOTE_EIGHTH },
    { NOTE_C, 4, NOTE_HALF },
};

static const MelodyStep_t s_twinkleSteps[] = {
    /* Twinkle, twinkle, little star */
    { NOTE_C, 4, NOTE_QUARTER }, { NOTE_C, 4, NOTE_QUARTER },
    { NOTE_G, 4, NOTE_QUARTER }, { NOTE_G, 4, NOTE_QUARTER },
    { NOTE_A, 4, NOTE_QUARTER }, { NOTE_A, 4, NOTE_QUARTER },
    { NOTE_G, 4, NOTE_HALF },
    /* How I wonder what you are */
    { NOTE_F, 4, NOTE_QUARTER }, { NOTE_F, 4, NOTE_QUARTER },
    { NOTE_E, 4, NOTE_QUARTER }, { NOTE_E, 4, NOTE_QUARTER },
    { NOTE_D, 4, NOTE_QUARTER }, { NOTE_D, 4, NOTE_QUARTER },
    { NOTE_C, 4, NOTE_HALF },
    /* Up above the world so high */
    { NOTE_G, 4, NOTE_QUARTER }, { NOTE_G, 4, NOTE_QUARTER },
    { NOTE_F, 4, NOTE_QUARTER }, { NOTE_F, 4, NOTE_QUARTER },
    { NOTE_E, 4, NOTE_QUARTER }, { NOTE_E, 4, NOTE_QUARTER },
    { NOTE_D, 4, NOTE_HALF },
    /* Like a diamond in the sky */
    { NOTE_G, 4, NOTE_QUARTER }, { NOTE_G, 4, NOTE_QUARTER },
    { NOTE_F, 4, NOTE_QUARTER }, { NOTE_F, 4, NOTE_QUARTER },
    { NOTE_E, 4, NOTE_QUARTER }, { NOTE_E, 4, NOTE_QUARTER },
    { NOTE_D, 4, NOTE_HALF },
    /* Twinkle, twinkle, little star */
    { NOTE_C, 4, NOTE_QUARTER }, { NOTE_C, 4, NOTE_QUARTER },
    { NOTE_G, 4, NOTE_QUARTER }, { NOTE_G, 4, NOTE_QUARTER },
    { NOTE_A, 4, NOTE_QUARTER }, { NOTE_A, 4, NOTE_QUARTER },
    { NOTE_G, 4, NOTE_HALF },
    /* How I wonder what you are */
    { NOTE_F, 4, NOTE_QUARTER }, { NOTE_F, 4, NOTE_QUARTER },
    { NOTE_E, 4, NOTE_QUARTER }, { NOTE_E, 4, NOTE_QUARTER },
    { NOTE_D, 4, NOTE_QUARTER }, { NOTE_D, 4, NOTE_QUARTER },
    { NOTE_C, 4, NOTE_HALF },
};

static const MelodyStep_t s_odeToJoySteps[] = {
    { NOTE_E, 4, NOTE_QUARTER }, { NOTE_E, 4, NOTE_QUARTER },
    { NOTE_F, 4, NOTE_QUARTER }, { NOTE_G, 4, NOTE_QUARTER },
    { NOTE_G, 4, NOTE_QUARTER }, { NOTE_F, 4, NOTE_QUARTER },
    { NOTE_E, 4, NOTE_QUARTER }, { NOTE_D, 4, NOTE_QUARTER },
    { NOTE_C, 4, NOTE_QUARTER }, { NOTE_C, 4, NOTE_QUARTER },
    { NOTE_D, 4, NOTE_QUARTER }, { NOTE_E, 4, NOTE_QUARTER },
    { NOTE_E, 4, NOTE_DOTTED_QUARTER },
    { NOTE_D, 4, NOTE_EIGHTH },
    { NOTE_D, 4, NOTE_HALF },
};

const Melody_t MELODY_C_MAJOR_SCALE = {
    .title     = "C major scale",
    .steps     = s_cMajorScaleSteps,
    .stepCount = MELODY_STEP_COUNT(s_cMajorScaleSteps),
    .tempoBpm  = 120U,
};

const Melody_t MELODY_TWINKLE = {
    .title     = "Twinkle, Twinkle, Little Star",
    .steps     = s_twinkleSteps,
    .stepCount = MELODY_STEP_COUNT(s_twinkleSteps),
    .tempoBpm  = 120U,
};

const Melody_t MELODY_ODE_TO_JOY = {
    .title     = "Ode to Joy",
    .steps     = s_odeToJoySteps,
    .stepCount = MELODY_STEP_COUNT(s_odeToJoySteps),
    .tempoBpm  = 100U,
};

/* --------------------------------------------------------------------------
 * Timing
 * -------------------------------------------------------------------------- */

uint32_t Melody_StepDurationMs(const Melody_t *melody, NoteLength_t length)
{
    if (melody == NULL || melody->tempoBpm == 0U) {
        return 0U;
    }

    return ((uint32_t)length * MELODY_MS_PER_MINUTE) /
           ((uint32_t)melody->tempoBpm * NOTE_SIXTEENTHS_PER_BEAT);
}
