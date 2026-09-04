/**
 ******************************************************************************
 * @file    ov7670_capture.h
 * @brief   DCMI capture control
 ******************************************************************************
 */

#ifndef OV7670_CAPTURE_H
#define OV7670_CAPTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ov7670_types.h"

/**
 * @brief Start a continuous DCMI capture into @p buffer.
 * @param length Buffer size in 32-bit words, as required by HAL_DCMI_Start_DMA.
 */
OV7670_StatusTypeDef OV7670_StartCapture(OV7670_Handle_t *hov7670, uint32_t *buffer,
                                         uint32_t length);
OV7670_StatusTypeDef OV7670_StopCapture(OV7670_Handle_t *hov7670);
bool OV7670_IsCaptureComplete(OV7670_Handle_t *hov7670);

#ifdef __cplusplus
}
#endif

#endif /* OV7670_CAPTURE_H */
