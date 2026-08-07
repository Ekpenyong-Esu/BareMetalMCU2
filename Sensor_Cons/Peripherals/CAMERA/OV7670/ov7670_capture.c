/**
  ******************************************************************************
  * @file    ov7670_capture.c
  * @brief   DCMI capture control
  ******************************************************************************
  */

#include "ov7670_capture.h"

OV7670_StatusTypeDef OV7670_StartCapture(OV7670_Handle_t *hov7670,
                                         uint32_t *buffer, uint32_t length)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }
    if (buffer == NULL || length == 0U || hov7670->hdcmi == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (HAL_DCMI_Start_DMA(hov7670->hdcmi, DCMI_MODE_CONTINUOUS,
                           (uint32_t)buffer, length) != HAL_OK) {
        return OV7670_ERROR;
    }

    return OV7670_OK;
}

OV7670_StatusTypeDef OV7670_StopCapture(OV7670_Handle_t *hov7670)
{
    OV7670_StatusTypeDef status = OV7670_CheckReady(hov7670);

    if (status != OV7670_OK) {
        return status;
    }
    if (hov7670->hdcmi == NULL) {
        return OV7670_INVALID_PARAM;
    }

    if (HAL_DCMI_Stop(hov7670->hdcmi) != HAL_OK) {
        return OV7670_ERROR;
    }

    return OV7670_OK;
}

bool OV7670_IsCaptureComplete(OV7670_Handle_t *hov7670)
{
    if (OV7670_CheckReady(hov7670) != OV7670_OK || hov7670->hdcmi == NULL) {
        return false;
    }

    return (HAL_DCMI_GetState(hov7670->hdcmi) == HAL_DCMI_STATE_READY);
}
