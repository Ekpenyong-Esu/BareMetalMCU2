/**
 ******************************************************************************
 * @file    accel_diag.c
 * @brief   Self-test and status reporting for the MMA8452Q
 ******************************************************************************
 */

#include "accel_diag.h"
#include "accel_data.h"
#include "accel_io.h"
#include "accel_mma8452q.h"
#include <math.h>
#include <stddef.h>

#define ACCEL_SELF_TEST_SETTLE_MS 50U
#define ACCEL_SELF_TEST_MIN_DELTA_G 0.5f

static const char *const s_statusNames[] = {
    [ACCEL_OK] = "ACCEL_OK",
    [ACCEL_ERROR] = "ACCEL_ERROR",
    [ACCEL_BUSY] = "ACCEL_BUSY",
    [ACCEL_TIMEOUT] = "ACCEL_TIMEOUT",
    [ACCEL_INVALID_PARAM] = "ACCEL_INVALID_PARAM",
    [ACCEL_NOT_READY] = "ACCEL_NOT_READY",
};

ACCEL_StatusTypeDef ACCEL_SelfTest(ACCEL_Handle_t *haccel) {
    ACCEL_DataTypeDef before;
    ACCEL_DataTypeDef after;

    ACCEL_StatusTypeDef status = ACCEL_ReadData(haccel, &before);
    if (status != ACCEL_OK) {
        return status;
    }

    status = ACCEL_UpdateRegister(haccel, ACCEL_REG_CTRL_REG2, ACCEL_CTRL_REG2_SELF_TEST,
                                  ACCEL_CTRL_REG2_SELF_TEST);
    if (status != ACCEL_OK) {
        return status;
    }

    HAL_Delay(ACCEL_SELF_TEST_SETTLE_MS);

    status = ACCEL_ReadData(haccel, &after);

    ACCEL_StatusTypeDef clearStatus =
        ACCEL_UpdateRegister(haccel, ACCEL_REG_CTRL_REG2, ACCEL_CTRL_REG2_SELF_TEST, 0x00U);
    if (status != ACCEL_OK) {
        return status;
    }
    if (clearStatus != ACCEL_OK) {
        return clearStatus;
    }

    /* Self-test injects an electrostatic force; any axis moving is a pass. */
    const bool moved = (fabsf(after.X_g - before.X_g) >= ACCEL_SELF_TEST_MIN_DELTA_G) ||
                       (fabsf(after.Y_g - before.Y_g) >= ACCEL_SELF_TEST_MIN_DELTA_G) ||
                       (fabsf(after.Z_g - before.Z_g) >= ACCEL_SELF_TEST_MIN_DELTA_G);

    return moved ? ACCEL_OK : ACCEL_ERROR;
}

const char *ACCEL_GetStatusString(ACCEL_StatusTypeDef status) {
    if ((size_t)status >= (sizeof(s_statusNames) / sizeof(s_statusNames[0]))) {
        return "UNKNOWN_STATUS";
    }

    return s_statusNames[status];
}
