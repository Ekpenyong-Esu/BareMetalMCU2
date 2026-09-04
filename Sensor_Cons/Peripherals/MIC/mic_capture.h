/**
 ******************************************************************************
 * @file    mic_capture.h
 * @brief   Recording control and PCM buffer readout
 ******************************************************************************
 */

#ifndef MIC_CAPTURE_H
#define MIC_CAPTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mic_types.h"

/**
 * @brief Reset the buffer and filter state, then start the circular DMA capture.
 */
MIC_StatusTypeDef MIC_StartRecording(MIC_HandleTypeDef *hmic);

MIC_StatusTypeDef MIC_StopRecording(MIC_HandleTypeDef *hmic);
MIC_StatusTypeDef MIC_PauseRecording(MIC_HandleTypeDef *hmic);
MIC_StatusTypeDef MIC_ResumeRecording(MIC_HandleTypeDef *hmic);

/**
 * @brief Copy the latest PCM block out, applying volume and gain.
 * @retval MIC_BUSY when no block has been captured since the last read.
 */
MIC_StatusTypeDef MIC_ReadAudioData(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length);

/**
 * @brief True while capture is running and not paused.
 */
bool MIC_IsRecording(const MIC_HandleTypeDef *hmic);

#ifdef __cplusplus
}
#endif

#endif /* MIC_CAPTURE_H */
