/**
 ******************************************************************************
 * @file    stepper_sequence.h
 * @brief   Coil excitation tables for the supported stepping modes
 ******************************************************************************
 */

#ifndef STEPPER_SEQUENCE_H
#define STEPPER_SEQUENCE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stepper_types.h"

/**
 * @brief  Number of entries in the sequence for @p mode.
 */
uint8_t STEPPER_SEQ_GetLength(STEPPER_StepMode_t mode);

/**
 * @brief  Coil states for one entry of the sequence.
 * @param  step Index; wrapped internally, so any value is accepted.
 * @return Array of STEPPER_COIL_COUNT flags, never NULL.
 */
const uint8_t *STEPPER_SEQ_GetPattern(STEPPER_StepMode_t mode, uint8_t step);

#ifdef __cplusplus
}
#endif

#endif /* STEPPER_SEQUENCE_H */
