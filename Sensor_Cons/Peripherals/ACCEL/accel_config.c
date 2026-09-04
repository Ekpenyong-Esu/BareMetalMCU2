/**
 ******************************************************************************
 * @file    accel_config.c
 * @brief   Runtime configuration of the MMA8452Q
 ******************************************************************************
 */

#include "accel_config.h"
#include "accel_io.h"
#include "accel_mma8452q.h"
#include <stddef.h>

typedef struct {
    uint8_t mode;
    uint8_t mask;
    uint8_t value;
} ACCEL_ModeEntry_t;

static const ACCEL_ModeEntry_t s_modes[] = {
    {ACCEL_MODE_STANDBY, ACCEL_CTRL_REG1_ACTIVE, 0x00U},
    {ACCEL_MODE_ACTIVE, ACCEL_CTRL_REG1_ACTIVE, ACCEL_CTRL_REG1_ACTIVE},
    {ACCEL_MODE_SLEEP, ACCEL_CTRL_REG1_F_READ, ACCEL_CTRL_REG1_F_READ},
};

/* CTRL_REG4 (enable) and CTRL_REG5 (route to INT1) take the same bit layout. */
static uint8_t ACCEL_BuildInterruptMask(const ACCEL_IntConfigTypeDef *config) {
    uint8_t mask = 0x00U;

    if (config->DataReady) {
        mask |= ACCEL_INT_BIT_DRDY;
    }
    if (config->Motion || config->Freefall) {
        mask |= ACCEL_INT_BIT_FF_MT;
    }
    if (config->Tap) {
        mask |= ACCEL_INT_BIT_PULSE;
    }

    return mask;
}

ACCEL_StatusTypeDef ACCEL_SetMode(ACCEL_Handle_t *haccel, uint8_t mode) {
    for (size_t i = 0; i < (sizeof(s_modes) / sizeof(s_modes[0])); i++) {
        if (s_modes[i].mode == mode) {
            return ACCEL_UpdateRegister(haccel, ACCEL_REG_CTRL_REG1, s_modes[i].mask,
                                        s_modes[i].value);
        }
    }

    return ACCEL_INVALID_PARAM;
}

ACCEL_StatusTypeDef ACCEL_GetMode(ACCEL_Handle_t *haccel, uint8_t *mode) {
    if (mode == NULL) {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t sysmod = 0;

    ACCEL_StatusTypeDef status = ACCEL_ReadRegister(haccel, ACCEL_REG_SYSMOD, &sysmod);
    if (status == ACCEL_OK) {
        *mode = sysmod & ACCEL_SYSMOD_MASK;
    }

    return status;
}

ACCEL_StatusTypeDef ACCEL_SetDataRate(ACCEL_Handle_t *haccel, uint8_t odr) {
    if (odr > ACCEL_ODR_1_56HZ) {
        return ACCEL_INVALID_PARAM;
    }

    return ACCEL_UpdateRegister(haccel, ACCEL_REG_CTRL_REG1, ACCEL_CTRL_REG1_ODR_MASK,
                                (uint8_t)(odr << ACCEL_CTRL_REG1_ODR_SHIFT));
}

ACCEL_StatusTypeDef ACCEL_SetRange(ACCEL_Handle_t *haccel, uint8_t range) {
    if (range > ACCEL_RANGE_8G) {
        return ACCEL_INVALID_PARAM;
    }

    ACCEL_StatusTypeDef status = ACCEL_WriteRegister(haccel, ACCEL_REG_XYZ_DATA_CFG, range);
    if (status == ACCEL_OK) {
        haccel->cachedRange = range;
    }

    return status;
}

ACCEL_StatusTypeDef ACCEL_GetRange(ACCEL_Handle_t *haccel, uint8_t *range) {
    if (range == NULL) {
        return ACCEL_INVALID_PARAM;
    }

    uint8_t cfg = 0;

    ACCEL_StatusTypeDef status = ACCEL_ReadRegister(haccel, ACCEL_REG_XYZ_DATA_CFG, &cfg);
    if (status == ACCEL_OK) {
        *range = cfg & ACCEL_XYZ_DATA_CFG_FS_MASK;
    }

    return status;
}

ACCEL_StatusTypeDef ACCEL_EnableHighPassFilter(ACCEL_Handle_t *haccel, bool enable) {
    return ACCEL_WriteRegister(haccel, ACCEL_REG_HP_FILTER_CUTOFF,
                               enable ? ACCEL_HP_FILTER_ENABLE : 0x00U);
}

ACCEL_StatusTypeDef ACCEL_EnableLowNoise(ACCEL_Handle_t *haccel, bool enable) {
    return ACCEL_UpdateRegister(haccel, ACCEL_REG_CTRL_REG1, ACCEL_CTRL_REG1_LNOISE,
                                enable ? ACCEL_CTRL_REG1_LNOISE : 0x00U);
}

ACCEL_StatusTypeDef ACCEL_ConfigInterrupts(ACCEL_Handle_t *haccel,
                                           const ACCEL_IntConfigTypeDef *config) {
    if (config == NULL) {
        return ACCEL_INVALID_PARAM;
    }

    const uint8_t mask = ACCEL_BuildInterruptMask(config);

    ACCEL_StatusTypeDef status = ACCEL_WriteRegister(haccel, ACCEL_REG_CTRL_REG4, mask);
    if (status != ACCEL_OK) {
        return status;
    }

    return ACCEL_WriteRegister(haccel, ACCEL_REG_CTRL_REG5, mask);
}

ACCEL_StatusTypeDef ACCEL_GetInterruptSource(ACCEL_Handle_t *haccel, uint8_t *intSource) {
    if (intSource == NULL) {
        return ACCEL_INVALID_PARAM;
    }

    return ACCEL_ReadRegister(haccel, ACCEL_REG_INT_SOURCE, intSource);
}
