/**
  ******************************************************************************
  * @file    ov7670_control.c
  * @brief   Runtime image controls
  ******************************************************************************
  */

#include "ov7670_control.h"
#include "ov7670_modes.h"
#include "ov7670_io.h"
#include "ov7670_regs.h"

#define OV7670_BRIGHTNESS_NEUTRAL       128
#define OV7670_LEVEL_TO_REG_DIVISOR     4U      /**< 0-255 control range to the 0-63 register range */

static const uint8_t s_testPatternBits[] = {
    [OV7670_TEST_PATTERN_NONE] = 0x00U,
    [OV7670_TEST_PATTERN_1]    = OV7670_XSC_TEST_PATTERN_1,
    [OV7670_TEST_PATTERN_2]    = OV7670_XSC_TEST_PATTERN_2,
    [OV7670_TEST_PATTERN_BARS] = OV7670_XSC_TEST_PATTERN_BARS,
};

#define OV7670_TEST_PATTERN_COUNT (sizeof(s_testPatternBits) / sizeof(s_testPatternBits[0]))

static OV7670_StatusTypeDef OV7670_SetFlagBit(OV7670_Handle_t *hov7670, uint8_t reg,
                                              uint8_t bit, bool enable)
{
    return OV7670_UpdateReg(hov7670, reg, bit, enable ? bit : 0U);
}

OV7670_StatusTypeDef OV7670_SetResolution(OV7670_Handle_t *hov7670,
                                          OV7670_ResolutionTypeDef resolution)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_ApplyResolution(hov7670, resolution);
    if (status == OV7670_OK) {
        hov7670->config.resolution = resolution;
    }

    return status;
}

OV7670_StatusTypeDef OV7670_SetFormat(OV7670_Handle_t *hov7670, OV7670_FormatTypeDef format)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_ApplyFormat(hov7670, format);
    if (status == OV7670_OK) {
        hov7670->config.format = format;
    }

    return status;
}

OV7670_StatusTypeDef OV7670_SetBrightness(OV7670_Handle_t *hov7670, uint8_t brightness)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);
    int8_t signed_level;

    if (status != OV7670_OK) {
        return status;
    }

    /* BRIGHT is signed: the 0-255 control range maps onto -128..+127. */
    signed_level = (int8_t)((int16_t)brightness - OV7670_BRIGHTNESS_NEUTRAL);

    status = OV7670_WriteReg(hov7670, OV7670_REG_BRIGHT, (uint8_t)signed_level);
    if (status == OV7670_OK) {
        hov7670->config.brightness = brightness;
    }

    return status;
}

OV7670_StatusTypeDef OV7670_SetContrast(OV7670_Handle_t *hov7670, uint8_t contrast)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_WriteReg(hov7670, OV7670_REG_CONTRAS,
                             (uint8_t)(contrast / OV7670_LEVEL_TO_REG_DIVISOR));
    if (status == OV7670_OK) {
        hov7670->config.contrast = contrast;
    }

    return status;
}

OV7670_StatusTypeDef OV7670_SetSaturation(OV7670_Handle_t *hov7670, uint8_t saturation)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);
    uint8_t level;

    if (status != OV7670_OK) {
        return status;
    }

    /* Saturation is scaled through the U and V colour-matrix coefficients. */
    level = (uint8_t)(saturation / OV7670_LEVEL_TO_REG_DIVISOR);

    status = OV7670_WriteReg(hov7670, OV7670_REG_MTX1, level);
    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_WriteReg(hov7670, OV7670_REG_MTX4, level);
    if (status == OV7670_OK) {
        hov7670->config.saturation = saturation;
    }

    return status;
}

OV7670_StatusTypeDef OV7670_SetFlipHorizontal(OV7670_Handle_t *hov7670, bool enable)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_SetFlagBit(hov7670, OV7670_REG_MVFP, OV7670_MVFP_MIRROR, enable);
    if (status == OV7670_OK) {
        hov7670->config.flip_horizontal = enable;
    }

    return status;
}

OV7670_StatusTypeDef OV7670_SetFlipVertical(OV7670_Handle_t *hov7670, bool enable)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_SetFlagBit(hov7670, OV7670_REG_MVFP, OV7670_MVFP_VFLIP, enable);
    if (status == OV7670_OK) {
        hov7670->config.flip_vertical = enable;
    }

    return status;
}

OV7670_StatusTypeDef OV7670_SetNightMode(OV7670_Handle_t *hov7670, bool enable)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }

    status = OV7670_SetFlagBit(hov7670, OV7670_REG_COM11, OV7670_COM11_NIGHT_MODE, enable);
    if (status == OV7670_OK) {
        hov7670->config.night_mode = enable;
    }

    return status;
}

OV7670_StatusTypeDef OV7670_SetTestPattern(OV7670_Handle_t *hov7670, uint8_t pattern)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }
    if (pattern >= OV7670_TEST_PATTERN_COUNT) {
        return OV7670_INVALID_PARAM;
    }

    status = OV7670_UpdateReg(hov7670, OV7670_REG_SCALING_XSC,
                              OV7670_XSC_TEST_PATTERN_MASK, s_testPatternBits[pattern]);
    if (status == OV7670_OK) {
        hov7670->config.test_pattern = pattern;
    }

    return status;
}
