/**
 ******************************************************************************
 * @file    ov7670_modes.c
 * @brief   Resolution and colour-format register programming
 ******************************************************************************
 */

#include "ov7670_modes.h"
#include "ov7670_io.h"
#include "ov7670_regs.h"

/* QVGA (320x240) RGB565 timing and scaling */
static const OV7670_RegVal_t s_qvgaRgb565[] = {
    {OV7670_REG_COM7, OV7670_COM7_FMT_QVGA | OV7670_COM7_FMT_RGB},
    {OV7670_REG_CLKRC, 0x01},
    {OV7670_REG_TSLB, 0x04},
    {OV7670_REG_COM10, 0x02},
    {OV7670_REG_HSTART, 0x16},
    {OV7670_REG_HSTOP, 0x04},
    {OV7670_REG_HREF, 0x24},
    {OV7670_REG_VSTART, 0x02},
    {OV7670_REG_VSTOP, 0x7a},
    {OV7670_REG_VREF, 0x0a},
    {OV7670_REG_COM3, 0x04},
    {OV7670_REG_COM14, 0x1a},
    {OV7670_REG_SCALING_XSC, 0x3a},
    {OV7670_REG_SCALING_YSC, 0x35},
    {OV7670_REG_SCALING_DCWCTR, 0x11},
    {OV7670_REG_SCALING_PCLK_DIV, 0xf0},
    {OV7670_REG_COM9, 0x2a},
    {OV7670_REG_COM16, 0x08},
    {OV7670_REG_COM17, 0x00},
    {OV7670_REG_COM1, 0x00},
    {OV7670_REG_COM2, 0x01},
    {OV7670_REG_COM15, OV7670_COM15_RGB565},
    {OV7670_REG_LIST_END, OV7670_REG_LIST_END}};

/* VGA (640x480) RGB565 timing, no downsampling */
static const OV7670_RegVal_t s_vgaRgb565[] = {
    {OV7670_REG_COM7, OV7670_COM7_FMT_VGA | OV7670_COM7_FMT_RGB},
    {OV7670_REG_CLKRC, 0x01},
    {OV7670_REG_TSLB, 0x04},
    {OV7670_REG_COM10, 0x02},
    {OV7670_REG_HSTART, 0x13},
    {OV7670_REG_HSTOP, 0x01},
    {OV7670_REG_HREF, 0xb6},
    {OV7670_REG_VSTART, 0x02},
    {OV7670_REG_VSTOP, 0x7a},
    {OV7670_REG_VREF, 0x0a},
    {OV7670_REG_COM3, 0x00},
    {OV7670_REG_COM14, 0x00},
    {OV7670_REG_SCALING_XSC, 0x3a},
    {OV7670_REG_SCALING_YSC, 0x35},
    {OV7670_REG_SCALING_DCWCTR, 0x11},
    {OV7670_REG_SCALING_PCLK_DIV, 0xf0},
    {OV7670_REG_COM9, 0x2a},
    {OV7670_REG_COM16, 0x08},
    {OV7670_REG_COM17, 0x00},
    {OV7670_REG_COM1, 0x00},
    {OV7670_REG_COM2, 0x01},
    {OV7670_REG_COM15, OV7670_COM15_RGB565},
    {OV7670_REG_LIST_END, OV7670_REG_LIST_END}};

/* QQVGA has no dedicated table: it is QVGA plus an extra downsample step. */
#define OV7670_DCWCTR_KEEP_TABLE_VALUE 0x00U
#define OV7670_DCWCTR_QQVGA 0x33U

static const struct {
    const OV7670_RegVal_t *regs;
    uint8_t dcwctrOverride;
} s_resolutions[OV7670_RES_COUNT] = {
    [OV7670_RES_QQVGA] = {s_qvgaRgb565, OV7670_DCWCTR_QQVGA},
    [OV7670_RES_QVGA] = {s_qvgaRgb565, OV7670_DCWCTR_KEEP_TABLE_VALUE},
    [OV7670_RES_VGA] = {s_vgaRgb565, OV7670_DCWCTR_KEEP_TABLE_VALUE},
};

/* The output format is the COM7 bit pair plus its COM15 companion. */
static const struct {
    uint8_t com7;
    uint8_t com15;
} s_formats[OV7670_FMT_COUNT] = {
    [OV7670_FMT_RGB565] = {OV7670_COM7_FMT_RGB, OV7670_COM15_RGB565},
    [OV7670_FMT_RGB555] = {OV7670_COM7_FMT_RGB, OV7670_COM15_RGB555},
    [OV7670_FMT_YUV422] = {OV7670_COM7_FMT_YUV, OV7670_COM15_YUV422},
    [OV7670_FMT_GRAYSCALE] = {OV7670_COM7_FMT_RAW, OV7670_COM15_RAW_BAYER},
};

OV7670_StatusTypeDef OV7670_ApplyResolution(OV7670_Handle_t *hov7670,
                                            OV7670_ResolutionTypeDef resolution) {
    OV7670_StatusTypeDef status = OV7670_OK;

    if (resolution >= OV7670_RES_COUNT) {
        return OV7670_INVALID_PARAM;
    }

    status = OV7670_WriteRegList(hov7670, s_resolutions[resolution].regs);
    if (status != OV7670_OK ||
        s_resolutions[resolution].dcwctrOverride == OV7670_DCWCTR_KEEP_TABLE_VALUE) {
        return status;
    }

    return OV7670_WriteReg(hov7670, OV7670_REG_SCALING_DCWCTR,
                           s_resolutions[resolution].dcwctrOverride);
}

OV7670_StatusTypeDef OV7670_ApplyFormat(OV7670_Handle_t *hov7670, OV7670_FormatTypeDef format) {
    OV7670_StatusTypeDef status = OV7670_OK;

    if (format >= OV7670_FMT_COUNT) {
        return OV7670_INVALID_PARAM;
    }

    status =
        OV7670_UpdateReg(hov7670, OV7670_REG_COM7, OV7670_COM7_FMT_MASK, s_formats[format].com7);
    if (status != OV7670_OK) {
        return status;
    }

    return OV7670_WriteReg(hov7670, OV7670_REG_COM15, s_formats[format].com15);
}
