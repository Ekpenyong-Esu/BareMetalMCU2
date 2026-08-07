/**
  ******************************************************************************
  * @file    ov7670_modes.h
  * @brief   Resolution and colour-format register programming
  ******************************************************************************
  */

#ifndef OV7670_MODES_H
#define OV7670_MODES_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ov7670_types.h"

OV7670_StatusTypeDef OV7670_ApplyResolution(OV7670_Handle_t *hov7670,
                                            OV7670_ResolutionTypeDef resolution);
OV7670_StatusTypeDef OV7670_ApplyFormat(OV7670_Handle_t *hov7670,
                                        OV7670_FormatTypeDef format);

#ifdef __cplusplus
}
#endif

#endif /* OV7670_MODES_H */
