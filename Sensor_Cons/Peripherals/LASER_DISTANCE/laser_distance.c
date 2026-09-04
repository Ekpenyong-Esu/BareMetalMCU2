/**
 ******************************************************************************
 * @file    laser_distance.c
 * @brief   Laser distance sensor lifecycle and configuration
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "laser_distance_core.h"
#include "laser_distance_measure.h"
#include "laser_distance_vl53l0x.h"
#include "i2c.h"
#include "log.h"
#include <string.h>

/* Private variables ---------------------------------------------------------*/

/** @brief One entry per LASER_DISTANCE_StatusTypeDef value, in enum order */
static const char *const statusStrings[] = {"OK",           "ERROR",         "BUSY",
                                            "TIMEOUT",      "INVALID_PARAM", "NOT_INITIALIZED",
                                            "OUT_OF_RANGE", "I2C_ERROR"};

/* Private function prototypes -----------------------------------------------*/
static LASER_DISTANCE_StatusTypeDef
LASER_DISTANCE_ValidateConfig(const LASER_DISTANCE_Config_t *config);

/* Public functions ----------------------------------------------------------*/

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Init(LASER_DISTANCE_Handle_t *hlaser, I2C_Bus_t *bus,
                                                 LASER_DISTANCE_SensorType_t sensorType) {
    LASER_DISTANCE_Config_t defaultConfig;
    LASER_DISTANCE_StatusTypeDef status = LASER_DISTANCE_OK;
    const I2C_ConfigTypeDef busConfig = I2C_ConfigDefault();

    if (hlaser == NULL || bus == NULL) {
        log_error("LASER_DISTANCE: Invalid parameters provided to LASER_DISTANCE_Init");
        return LASER_DISTANCE_INVALID_PARAM;
    }

    log_debug("LASER_DISTANCE: Initializing laser distance sensor with sensor type %d", sensorType);

    memset(hlaser, 0, sizeof(LASER_DISTANCE_Handle_t));

    /* A rejected configuration would leave the zeroed handle in place, and
       every later transfer would then address I2C slave 0. */
    defaultConfig = LASER_DISTANCE_GetDefaultConfig(sensorType);
    status = LASER_DISTANCE_Config(hlaser, &defaultConfig);
    if (status != LASER_DISTANCE_OK) {
        log_error("LASER_DISTANCE: Default configuration rejected for sensor type %d", sensorType);
        return status;
    }

    /* Registered once; the address is validated by the config above. */
    if (I2C_DeviceInit(&hlaser->device, bus, hlaser->config.i2cAddress, &busConfig) != I2C_OK) {
        log_error("LASER_DISTANCE: I2C device registration failed");
        return LASER_DISTANCE_I2C_ERROR;
    }

    switch (sensorType) {
        case LASER_DISTANCE_VL53L0X:
            status = LASER_DISTANCE_VL53L0X_Init(hlaser);
            break;
        case LASER_DISTANCE_VL53L1X:
            log_warning("LASER_DISTANCE: VL53L1X not implemented yet");
            status = LASER_DISTANCE_ERROR;
            break;
        case LASER_DISTANCE_TFMINI:
            /* TFmini streams over UART and needs no register set-up */
            log_debug("LASER_DISTANCE: TFmini sensor initialized");
            status = LASER_DISTANCE_OK;
            break;
        case LASER_DISTANCE_CUSTOM:
            log_debug("LASER_DISTANCE: Custom sensor initialized");
            status = LASER_DISTANCE_OK;
            break;
        default:
            log_error("LASER_DISTANCE: Invalid sensor type %d", sensorType);
            status = LASER_DISTANCE_INVALID_PARAM;
            break;
    }

    if (status == LASER_DISTANCE_OK) {
        hlaser->isInitialized = true;
        log_info("LASER_DISTANCE: Laser distance sensor initialized successfully");
    }
    else {
        log_error("LASER_DISTANCE: Failed to initialize sensor, status: %d", status);
    }

    return status;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_DeInit(LASER_DISTANCE_Handle_t *hlaser) {
    if (hlaser == NULL) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    (void)LASER_DISTANCE_StopContinuous(hlaser);

    hlaser->isInitialized = false;

    return LASER_DISTANCE_OK;
}

LASER_DISTANCE_StatusTypeDef LASER_DISTANCE_Config(LASER_DISTANCE_Handle_t *hlaser,
                                                   const LASER_DISTANCE_Config_t *config) {
    LASER_DISTANCE_StatusTypeDef status = LASER_DISTANCE_OK;

    if (hlaser == NULL || config == NULL) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    status = LASER_DISTANCE_ValidateConfig(config);
    if (status != LASER_DISTANCE_OK) {
        return status;
    }

    hlaser->config = *config;

    /* The device record must answer to whatever address the config names,
       or a later transfer would go to the previous address. */
    hlaser->device.address = config->i2cAddress;

    return LASER_DISTANCE_OK;
}

LASER_DISTANCE_Config_t LASER_DISTANCE_GetDefaultConfig(LASER_DISTANCE_SensorType_t sensorType) {
    LASER_DISTANCE_Config_t config = {.sensorType = sensorType,
                                      .averagingSamples = LASER_DISTANCE_DEFAULT_AVERAGING_SAMPLES,
                                      .measurementTimeout =
                                          LASER_DISTANCE_DEFAULT_MEASUREMENT_TIMEOUT,
                                      .i2cAddress = LASER_DISTANCE_DEFAULT_I2C_ADDRESS};

    switch (sensorType) {
        case LASER_DISTANCE_VL53L0X:
            config.minDistance = LASER_DISTANCE_VL53L0X_MIN;
            config.maxDistance = LASER_DISTANCE_VL53L0X_MAX;
            break;
        case LASER_DISTANCE_VL53L1X:
            config.minDistance = LASER_DISTANCE_VL53L1X_MIN;
            config.maxDistance = LASER_DISTANCE_VL53L1X_MAX;
            break;
        case LASER_DISTANCE_TFMINI:
            config.minDistance = LASER_DISTANCE_TFMINI_MIN;
            config.maxDistance = LASER_DISTANCE_TFMINI_MAX;
            break;
        case LASER_DISTANCE_CUSTOM:
        default:
            config.minDistance = LASER_DISTANCE_CUSTOM_MIN;
            config.maxDistance = LASER_DISTANCE_CUSTOM_MAX;
            break;
    }

    return config;
}

bool LASER_DISTANCE_IsValidDistance(const LASER_DISTANCE_Handle_t *hlaser, uint16_t distance) {
    if (hlaser == NULL) {
        return false;
    }

    return (distance >= hlaser->config.minDistance && distance <= hlaser->config.maxDistance);
}

const char *LASER_DISTANCE_GetStatusString(LASER_DISTANCE_StatusTypeDef status) {
    if ((size_t)status >= (sizeof(statusStrings) / sizeof(statusStrings[0]))) {
        return "UNKNOWN";
    }

    return statusStrings[status];
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief   Validate configuration
 * @details The I2C address is held in 8-bit (shifted) form, so it must be even
 *          and its 7-bit part must be a legal slave address.
 * @param   config Pointer to configuration structure
 * @retval  LASER_DISTANCE_StatusTypeDef Validation status
 */
static LASER_DISTANCE_StatusTypeDef
LASER_DISTANCE_ValidateConfig(const LASER_DISTANCE_Config_t *config) {
    const uint8_t address7bit = (uint8_t)(config->i2cAddress >> 1);

    if (config->minDistance >= config->maxDistance) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    if (config->averagingSamples == 0U ||
        config->averagingSamples > LASER_DISTANCE_MAX_AVERAGING_SAMPLES) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    if (config->measurementTimeout == 0U) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    if ((config->i2cAddress & 0x01U) != 0U || address7bit < LASER_DISTANCE_MIN_7BIT_ADDRESS ||
        address7bit > LASER_DISTANCE_MAX_7BIT_ADDRESS) {
        return LASER_DISTANCE_INVALID_PARAM;
    }

    return LASER_DISTANCE_OK;
}
