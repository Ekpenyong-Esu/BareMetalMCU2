/**
  ******************************************************************************
  * @file    ov7670_control.h
  * @brief   Runtime image controls
  ******************************************************************************
  */

#ifndef OV7670_CONTROL_H
#define OV7670_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ov7670_types.h"

OV7670_StatusTypeDef OV7670_SetResolution(OV7670_Handle_t *hov7670,
                                          OV7670_ResolutionTypeDef resolution);
OV7670_StatusTypeDef OV7670_SetFormat(OV7670_Handle_t *hov7670, OV7670_FormatTypeDef format);
OV7670_StatusTypeDef OV7670_SetBrightness(OV7670_Handle_t *hov7670, uint8_t brightness);
OV7670_StatusTypeDef OV7670_SetContrast(OV7670_Handle_t *hov7670, uint8_t contrast);
OV7670_StatusTypeDef OV7670_SetSaturation(OV7670_Handle_t *hov7670, uint8_t saturation);
OV7670_StatusTypeDef OV7670_SetFlipHorizontal(OV7670_Handle_t *hov7670, bool enable);
OV7670_StatusTypeDef OV7670_SetFlipVertical(OV7670_Handle_t *hov7670, bool enable);
OV7670_StatusTypeDef OV7670_SetNightMode(OV7670_Handle_t *hov7670, bool enable);
OV7670_StatusTypeDef OV7670_SetTestPattern(OV7670_Handle_t *hov7670, uint8_t pattern);

#ifdef __cplusplus
}
#endif

#endif /* OV7670_CONTROL_H */
