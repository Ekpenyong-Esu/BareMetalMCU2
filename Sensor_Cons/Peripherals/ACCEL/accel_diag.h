/**
 ******************************************************************************
 * @file    accel_diag.h
 * @brief   Self-test and status reporting for the MMA8452Q
 ******************************************************************************
 */

#ifndef ACCEL_DIAG_H
#define ACCEL_DIAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include "accel_types.h"

/**
 * @brief Toggle the built-in self-test and check that the output moves.
 */
ACCEL_StatusTypeDef ACCEL_SelfTest(ACCEL_Handle_t *haccel);

const char *ACCEL_GetStatusString(ACCEL_StatusTypeDef status);

#ifdef __cplusplus
}
#endif

#endif /* ACCEL_DIAG_H */
