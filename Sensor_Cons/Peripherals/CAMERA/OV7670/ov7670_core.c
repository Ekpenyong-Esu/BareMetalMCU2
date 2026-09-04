/**
 ******************************************************************************
 * @file    ov7670_core.c
 * @brief   Lifecycle for the OV7670 camera driver
 ******************************************************************************
 */

#include "ov7670_core.h"
#include "ov7670_control.h"
#include "ov7670_capture.h"
#include "ov7670_io.h"
#include "ov7670_regs.h"

#define OV7670_DEFAULT_LEVEL 128U /**< Neutral brightness/contrast/saturation */

OV7670_StatusTypeDef OV7670_GetDefaultConfig(OV7670_Config_t *config) {
    if (config == NULL) {
        return OV7670_INVALID_PARAM;
    }

    config->resolution = OV7670_RES_QVGA;
    config->format = OV7670_FMT_RGB565;
    config->brightness = OV7670_DEFAULT_LEVEL;
    config->contrast = OV7670_DEFAULT_LEVEL;
    config->saturation = OV7670_DEFAULT_LEVEL;
    config->flip_horizontal = false;
    config->flip_vertical = false;
    config->night_mode = false;
    config->test_pattern = OV7670_TEST_PATTERN_NONE;

    return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_GetChipID(OV7670_Handle_t *hov7670, uint16_t *chip_id) {
    OV7670_StatusTypeDef status = OV7670_OK;
    uint8_t pid = 0;
    uint8_t ver = 0;

    if (hov7670 == NULL || chip_id == NULL) {
        return OV7670_INVALID_PARAM;
    }

    status = OV7670_ReadReg(hov7670, OV7670_REG_PID, &pid);
    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_ReadReg(hov7670, OV7670_REG_VER, &ver);
    if (status != OV7670_OK) {
        return status;
    }

    *chip_id = (uint16_t)(((uint16_t)pid << 8) | ver);

    return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_Config(OV7670_Handle_t *hov7670, const OV7670_Config_t *config) {
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }
    if (config == NULL) {
        return OV7670_INVALID_PARAM;
    }

    status = OV7670_SetResolution(hov7670, config->resolution);
    if (status == OV7670_OK) {
        status = OV7670_SetFormat(hov7670, config->format);
    }
    if (status == OV7670_OK) {
        status = OV7670_SetBrightness(hov7670, config->brightness);
    }
    if (status == OV7670_OK) {
        status = OV7670_SetContrast(hov7670, config->contrast);
    }
    if (status == OV7670_OK) {
        status = OV7670_SetSaturation(hov7670, config->saturation);
    }
    if (status == OV7670_OK) {
        status = OV7670_SetFlipHorizontal(hov7670, config->flip_horizontal);
    }
    if (status == OV7670_OK) {
        status = OV7670_SetFlipVertical(hov7670, config->flip_vertical);
    }
    if (status == OV7670_OK) {
        status = OV7670_SetNightMode(hov7670, config->night_mode);
    }
    if (status == OV7670_OK) {
        status = OV7670_SetTestPattern(hov7670, config->test_pattern);
    }

    return status;
}

OV7670_StatusTypeDef OV7670_Reset(OV7670_Handle_t *hov7670) {
    OV7670_StatusTypeDef status = OV7670_OK;

    if (hov7670 == NULL) {
        return OV7670_INVALID_PARAM;
    }

    status = OV7670_WriteReg(hov7670, OV7670_REG_COM7, OV7670_COM7_RESET);
    if (status != OV7670_OK) {
        return status;
    }

    HAL_Delay(OV7670_RESET_DELAY);

    return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_Init(OV7670_Handle_t *hov7670, DCMI_HandleTypeDef *hdcmi,
                                 I2C_Bus_t *bus) {
    OV7670_StatusTypeDef status = OV7670_OK;
    OV7670_Config_t default_config;
    I2C_ConfigTypeDef sccbConfig;
    uint16_t chip_id = 0;

    if (hov7670 == NULL || hdcmi == NULL || bus == NULL) {
        return OV7670_INVALID_PARAM;
    }

    hov7670->hdcmi = hdcmi;
    hov7670->initialized = false;
    hov7670->chip_id = 0;

    sccbConfig = I2C_ConfigDefault();
    sccbConfig.ClockSpeed = OV7670_SCCB_CLOCK_HZ;
    if (I2C_DeviceInit(&hov7670->device, bus, OV7670_I2C_ADDRESS, &sccbConfig) != I2C_OK) {
        return OV7670_I2C_ERROR;
    }

    status = OV7670_GetChipID(hov7670, &chip_id);
    if (status != OV7670_OK) {
        return status;
    }

    /* Only the product ID identifies the part; VER is the silicon revision. */
    if ((uint8_t)(chip_id >> 8) != OV7670_PID_VALUE) {
        return OV7670_INVALID_ID;
    }

    hov7670->chip_id = chip_id;

    status = OV7670_Reset(hov7670);
    if (status != OV7670_OK) {
        return status;
    }

    (void)OV7670_GetDefaultConfig(&default_config);

    /* Marked ready first: the setters OV7670_Config() drives require it. */
    hov7670->initialized = true;

    status = OV7670_Config(hov7670, &default_config);
    if (status != OV7670_OK) {
        hov7670->initialized = false;
        return status;
    }

    hov7670->config = default_config;

    return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_DeInit(OV7670_Handle_t *hov7670) {
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }

    (void)OV7670_StopCapture(hov7670);
    (void)OV7670_WriteReg(hov7670, OV7670_REG_COM2, OV7670_COM2_SOFT_SLEEP);

    hov7670->initialized = false;

    return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_GetStatus(OV7670_Handle_t *hov7670) {
    return OV7670_CheckReady(hov7670);
}
