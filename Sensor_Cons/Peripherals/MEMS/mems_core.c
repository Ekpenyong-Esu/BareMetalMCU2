/**
 ******************************************************************************
 * @file    mems_core.c
 * @brief   Lifecycle for the L3GD20 MEMS gyroscope driver
 ******************************************************************************
 */

#include "mems_core.h"
#include "mems_hw.h"
#include "mems_io.h"
#include "mems_gyro.h"
#include "mems_l3gd20.h"
#include <string.h>

#define MEMS_CS_SETTLE_MS 1U
#define MEMS_RETRY_DELAY_MS 10U

static const uint8_t s_resetRegisters[][2] = {
    {L3GD20_CTRL_REG1_ADDR, L3GD20_CTRL_REG1_RESET}, {L3GD20_CTRL_REG2_ADDR, L3GD20_CTRL_REG_RESET},
    {L3GD20_CTRL_REG3_ADDR, L3GD20_CTRL_REG_RESET},  {L3GD20_CTRL_REG4_ADDR, L3GD20_CTRL_REG_RESET},
    {L3GD20_CTRL_REG5_ADDR, L3GD20_CTRL_REG_RESET},
};

#define MEMS_RESET_REGISTER_COUNT (sizeof(s_resetRegisters) / sizeof(s_resetRegisters[0]))

static MEMS_StatusTypeDef MEMS_VerifyDevice(MEMS_HandleTypeDef *hmems) {
    uint8_t who_am_i = 0U;

    for (uint8_t retry = 0U; retry < MEMS_MAX_RETRIES; retry++) {
        if (MEMS_ReadRegister(hmems, L3GD20_WHO_AM_I_ADDR, &who_am_i) == MEMS_OK &&
            who_am_i == L3GD20_WHO_AM_I_VALUE) {
            return MEMS_OK;
        }
        HAL_Delay(MEMS_RETRY_DELAY_MS);
    }

    return MEMS_DEVICE_NOT_FOUND;
}

MEMS_StatusTypeDef MEMS_GetDefaultConfig(MEMS_GyroConfigTypeDef *config) {
    if (config == NULL) {
        return MEMS_INVALID_PARAM;
    }

    config->OutputDataRate = MEMS_GYRO_ODR_95Hz;
    config->FullScale = MEMS_GYRO_FULLSCALE_250;
    config->Bandwidth = MEMS_GYRO_BANDWIDTH_1;
    config->XAxisEnable = true;
    config->YAxisEnable = true;
    config->ZAxisEnable = true;
    config->PowerDownMode = false;

    return MEMS_OK;
}

/**
 * @brief Bus settings the part requires, which is why they are not configurable.
 * @note  Mode 0 and at most 10 MHz; PCLK/16 keeps the clock inside that on
 *        every APB the part could sit on.
 */
static SPI_ConfigTypeDef MEMS_BusConfig(void) {
    SPI_ConfigTypeDef config = SPI_ConfigDefault();

    config.CLKPolarity = SPI_POLARITY_LOW;
    config.CLKPhase = SPI_PHASE_1EDGE;
    config.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;

    return config;
}

MEMS_StatusTypeDef MEMS_Init(MEMS_HandleTypeDef *hmems, const MEMS_Config_t *config) {
    MEMS_StatusTypeDef status = MEMS_OK;
    MEMS_GyroConfigTypeDef default_config;
    SPI_ConfigTypeDef busConfig;

    if (hmems == NULL || config == NULL || config->Bus == NULL || config->CS_Port == NULL ||
        config->CS_Pin == 0U) {
        return MEMS_INVALID_PARAM;
    }

    memset(hmems, 0, sizeof(*hmems));
    hmems->CS_Port = config->CS_Port;
    hmems->CS_Pin = config->CS_Pin;
    hmems->INT1_Port = config->INT1_Port;
    hmems->INT1_Pin = config->INT1_Pin;
    hmems->INT2_Port = config->INT2_Port;
    hmems->INT2_Pin = config->INT2_Pin;

    status = MEMS_HW_InitGPIO(hmems);
    if (status != MEMS_OK) {
        return status;
    }

    busConfig = MEMS_BusConfig();
    if (SPI_DeviceInit(&hmems->device, config->Bus, &busConfig) != SPI_OK) {
        return MEMS_COMMUNICATION_ERROR;
    }

    HAL_Delay(MEMS_CS_SETTLE_MS);

    status = MEMS_VerifyDevice(hmems);
    if (status != MEMS_OK) {
        return status;
    }

    (void)MEMS_GetDefaultConfig(&default_config);

    /* Applied before the handle is marked ready, so the unguarded variant. */
    status = MEMS_GyroApplyConfig(hmems, &default_config);
    if (status != MEMS_OK) {
        return status;
    }

    hmems->IsInitialized = true;

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_DeInit(MEMS_HandleTypeDef *hmems) {
    if (hmems == NULL) {
        return MEMS_INVALID_PARAM;
    }

    (void)MEMS_SetPowerMode(hmems, true);

    hmems->IsInitialized = false;
    hmems->IsCalibrated = false;

    return MEMS_OK;
}

MEMS_StatusTypeDef MEMS_Reset(MEMS_HandleTypeDef *hmems) {
    MEMS_StatusTypeDef status = MEMS_OK;

    if (hmems == NULL) {
        return MEMS_INVALID_PARAM;
    }

    for (size_t i = 0U; i < MEMS_RESET_REGISTER_COUNT; i++) {
        status = MEMS_WriteRegister(hmems, s_resetRegisters[i][0], s_resetRegisters[i][1]);
        if (status != MEMS_OK) {
            return status;
        }
    }

    hmems->IsCalibrated = false;
    memset(&hmems->CalibrationOffset, 0, sizeof(hmems->CalibrationOffset));

    return MEMS_OK;
}
