/**
 ******************************************************************************
 * @file    stepper_sequence.c
 * @brief   Coil excitation tables for the supported stepping modes
 ******************************************************************************
 */

#include "stepper_sequence.h"

#define STEPPER_FULL_STEP_LENGTH 4U
#define STEPPER_HALF_STEP_LENGTH 8U
#define STEPPER_WAVE_STEP_LENGTH 4U

/* Both coils energised: highest torque */
static const uint8_t fullStepSequence[STEPPER_FULL_STEP_LENGTH][STEPPER_COIL_COUNT] = {
    {1, 1, 0, 0}, {0, 1, 1, 0}, {0, 0, 1, 1}, {1, 0, 0, 1}};

/* Alternating one- and two-coil states: double resolution */
static const uint8_t halfStepSequence[STEPPER_HALF_STEP_LENGTH][STEPPER_COIL_COUNT] = {
    {1, 0, 0, 0}, {1, 1, 0, 0}, {0, 1, 0, 0}, {0, 1, 1, 0},
    {0, 0, 1, 0}, {0, 0, 1, 1}, {0, 0, 0, 1}, {1, 0, 0, 1}};

/* One coil at a time: lowest current, lowest torque */
static const uint8_t waveDriveSequence[STEPPER_WAVE_STEP_LENGTH][STEPPER_COIL_COUNT] = {
    {1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

uint8_t STEPPER_SEQ_GetLength(STEPPER_StepMode_t mode) {
    switch (mode) {
        case STEPPER_MODE_HALF_STEP:
            return STEPPER_HALF_STEP_LENGTH;
        case STEPPER_MODE_WAVE_DRIVE:
            return STEPPER_WAVE_STEP_LENGTH;
        case STEPPER_MODE_FULL_STEP:
        default:
            return STEPPER_FULL_STEP_LENGTH;
    }
}

const uint8_t *STEPPER_SEQ_GetPattern(STEPPER_StepMode_t mode, uint8_t step) {
    uint8_t index = step % STEPPER_SEQ_GetLength(mode);

    switch (mode) {
        case STEPPER_MODE_HALF_STEP:
            return halfStepSequence[index];
        case STEPPER_MODE_WAVE_DRIVE:
            return waveDriveSequence[index];
        case STEPPER_MODE_FULL_STEP:
        default:
            return fullStepSequence[index];
    }
}
