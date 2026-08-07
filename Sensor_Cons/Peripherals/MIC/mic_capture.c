/**
  ******************************************************************************
  * @file    mic_capture.c
  * @brief   Recording control and PCM buffer readout
  ******************************************************************************
  */

#include "mic_capture.h"
#include "mic_core.h"
#include "mic_dsp.h"
#include <string.h>

MIC_StatusTypeDef MIC_StartRecording(MIC_HandleTypeDef *hmic)
{
    MIC_StatusTypeDef status = MIC_CheckReady(hmic);
    if (status != MIC_OK) {
        return status;
    }

    if (hmic->IsRecording) {
        return MIC_BUSY;
    }

    hmic->AudioBuffer.Position = 0;
    hmic->AudioBuffer.IsFull = false;
    hmic->BufferReady = false;
    hmic->HpPrevInput = 0;
    hmic->HpPrevOutput = 0;
    (void)MIC_ResetStatistics(hmic);

    if (HAL_I2S_Receive_DMA(hmic->hi2s, (uint16_t *)hmic->PDMBuffer, MIC_PDM_BUFFER_SIZE) != HAL_OK) {
        return MIC_DMA_ERROR;
    }

    hmic->IsRecording = true;
    hmic->IsPaused = false;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_StopRecording(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->IsRecording) {
        return MIC_OK;
    }

    HAL_I2S_DMAStop(hmic->hi2s);

    hmic->IsRecording = false;
    hmic->IsPaused = false;
    hmic->BufferReady = false;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_PauseRecording(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->IsRecording || hmic->IsPaused) {
        return MIC_OK;
    }

    HAL_I2S_DMAPause(hmic->hi2s);
    hmic->IsPaused = true;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_ResumeRecording(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->IsRecording || !hmic->IsPaused) {
        return MIC_OK;
    }

    HAL_I2S_DMAResume(hmic->hi2s);
    hmic->IsPaused = false;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_ReadAudioData(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length)
{
    MIC_StatusTypeDef status = MIC_CheckReady(hmic);
    if (status != MIC_OK) {
        return status;
    }

    if (buffer == NULL || length == 0) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->BufferReady) {
        return MIC_BUSY;
    }

    uint32_t copy_length = (length < MIC_PCM_SAMPLES) ? length : MIC_PCM_SAMPLES;
    memcpy(buffer, hmic->PCMBuffer, copy_length * sizeof(int16_t));

    MIC_ApplyScale(buffer, copy_length, (float)hmic->Config.Volume * MIC_VOLUME_SCALE_FACTOR);
    MIC_ApplyScale(buffer, copy_length, MIC_GainFactor(hmic->Config.Gain));
    MIC_UpdateStatistics(hmic, buffer, copy_length);

    hmic->BufferReady = false;

    return MIC_OK;
}

bool MIC_IsRecording(const MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return false;
    }

    return hmic->IsRecording && !hmic->IsPaused;
}
