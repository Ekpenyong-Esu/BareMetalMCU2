/**
 ******************************************************************************
 * @file    accel_core.c
 * @brief   Lifecycle and identity for the MMA8452Q accelerometer
 ******************************************************************************
 */

#include "accel_core.h"
#include "accel_config.h"
#include "accel_io.h"
#include "accel_mma8452q.h"
#include <string.h>

static const ACCEL_ConfigTypeDef s_defaultConfig = {.DataRate = ACCEL_ODR_100HZ,
                                                    .Range = ACCEL_RANGE_2G,
                                                    .Mode = ACCEL_MODE_ACTIVE,
                                                    .HighPassFilter = false,
                                                    .LowNoise = false};

ACCEL_StatusTypeDef ACCEL_GetDeviceID(ACCEL_Handle_t *haccel, uint8_t *deviceId) {
    if (deviceId == NULL) {
        return ACCEL_INVALID_PARAM;
    }

    return ACCEL_ReadRegister(haccel, ACCEL_REG_WHO_AM_I, deviceId);
}

ACCEL_StatusTypeDef ACCEL_IsReady(ACCEL_Handle_t *haccel) {
    uint8_t deviceId = 0;

    ACCEL_StatusTypeDef status = ACCEL_GetDeviceID(haccel, &deviceId);
    if (status != ACCEL_OK) {
        return status;
    }

    return (deviceId == ACCEL_DEVICE_ID) ? ACCEL_OK : ACCEL_NOT_READY;
}

static ACCEL_StatusTypeDef ACCEL_ApplyConfig(ACCEL_Handle_t *haccel) {
    const ACCEL_ConfigTypeDef *config = &haccel->config;

    /* Range and data rate are only writable while the device is in standby. */
    ACCEL_StatusTypeDef status = ACCEL_WriteRegister(haccel, ACCEL_REG_CTRL_REG1, 0x00U);
    if (status != ACCEL_OK) {
        return status;
    }

    status = ACCEL_SetRange(haccel, config->Range);
    if (status != ACCEL_OK) {
        return status;
    }

    const uint8_t ctrlReg1 = (uint8_t)((config->DataRate << ACCEL_CTRL_REG1_ODR_SHIFT) |
                                       (config->LowNoise ? ACCEL_CTRL_REG1_LNOISE : 0x00U));
    status = ACCEL_WriteRegister(haccel, ACCEL_REG_CTRL_REG1, ctrlReg1);
    if (status != ACCEL_OK) {
        return status;
    }

    if (config->HighPassFilter) {
        status = ACCEL_EnableHighPassFilter(haccel, true);
        if (status != ACCEL_OK) {
            return status;
        }
    }

    return ACCEL_SetMode(haccel, config->Mode);
}

ACCEL_StatusTypeDef ACCEL_Init(ACCEL_Handle_t *haccel, SPI_Bus_t *bus, GPIO_TypeDef *csPort,
                               uint16_t csPin, const ACCEL_ConfigTypeDef *config) {
    if (haccel == NULL || bus == NULL || csPort == NULL || csPin == 0U) {
        return ACCEL_INVALID_PARAM;
    }

    memset(haccel, 0, sizeof(*haccel));
    haccel->csPort = csPort;
    haccel->csPin = csPin;
    haccel->config = (config != NULL) ? *config : s_defaultConfig;
    haccel->cachedRange = ACCEL_RANGE_2G;

    ACCEL_StatusTypeDef status = ACCEL_IO_Init(haccel, bus);
    if (status != ACCEL_OK) {
        return status;
    }

    status = ACCEL_IsReady(haccel);
    if (status != ACCEL_OK) {
        return status;
    }

    status = ACCEL_ApplyConfig(haccel);
    if (status != ACCEL_OK) {
        return status;
    }

    haccel->isInitialized = true;

    return ACCEL_OK;
}

ACCEL_StatusTypeDef ACCEL_DeInit(ACCEL_Handle_t *haccel) {
    if (haccel == NULL) {
        return ACCEL_INVALID_PARAM;
    }

    ACCEL_StatusTypeDef status = ACCEL_SetMode(haccel, ACCEL_MODE_STANDBY);

    haccel->isInitialized = false;

    return status;
}
