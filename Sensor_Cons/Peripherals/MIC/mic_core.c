/**
  ******************************************************************************
  * @file    mic_core.c
  * @brief   Microphone lifecycle, configuration and handle registry
  ******************************************************************************
  */

#include "mic_core.h"
#include "mic_hw.h"
#include "mic_capture.h"
#include <string.h>

static MIC_HandleTypeDef *s_handle = NULL;

MIC_HandleTypeDef *MIC_GetHandle(void)
{
    return s_handle;
}

MIC_StatusTypeDef MIC_GetDefaultConfig(MIC_ConfigTypeDef *config)
{
    if (config == NULL) {
        return MIC_INVALID_PARAM;
    }

    config->SampleRate = MIC_SAMPLE_RATE_16KHZ;
    config->Gain = MIC_GAIN_0DB;
    config->Volume = MIC_DEFAULT_VOLUME;
    config->NoiseGateEnable = false;
    config->NoiseGateThreshold = MIC_NOISE_FLOOR;
    config->HighPassFilter = true;
    config->BufferSize = MIC_PCM_SAMPLES;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_Init(MIC_HandleTypeDef *hmic, I2S_HandleTypeDef *hi2s, DMA_HandleTypeDef *hdma)
{
    if (hmic == NULL || hi2s == NULL || hdma == NULL) {
        return MIC_INVALID_PARAM;
    }

    memset(hmic, 0, sizeof(MIC_HandleTypeDef));
    hmic->hi2s = hi2s;
    hmic->hdma = hdma;

    MIC_StatusTypeDef status = MIC_HW_InitGPIO();
    if (status != MIC_OK) {
        return status;
    }

    status = MIC_HW_InitI2S(hmic);
    if (status != MIC_OK) {
        return status;
    }

    status = MIC_HW_InitDMA(hmic);
    if (status != MIC_OK) {
        return status;
    }

    /* Registered before MIC_Configure() so the ISR path can already resolve the handle */
    s_handle = hmic;
    hmic->IsInitialized = true;

    MIC_ConfigTypeDef defaults;
    (void)MIC_GetDefaultConfig(&defaults);

    status = MIC_Configure(hmic, &defaults);
    if (status != MIC_OK) {
        hmic->IsInitialized = false;
        s_handle = NULL;
        return status;
    }

    return MIC_OK;
}

MIC_StatusTypeDef MIC_DeInit(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (hmic->IsRecording) {
        (void)MIC_StopRecording(hmic);
    }

    if (hmic->hi2s != NULL) {
        HAL_I2S_DeInit(hmic->hi2s);
    }

    if (hmic->hdma != NULL) {
        HAL_DMA_DeInit(hmic->hdma);
    }

    hmic->IsInitialized = false;
    hmic->IsRecording = false;

    if (s_handle == hmic) {
        s_handle = NULL;
    }

    return MIC_OK;
}

MIC_StatusTypeDef MIC_Configure(MIC_HandleTypeDef *hmic, const MIC_ConfigTypeDef *config)
{
    MIC_StatusTypeDef status = MIC_CheckReady(hmic);
    if (status != MIC_OK) {
        return status;
    }

    if (config == NULL || config->Volume > MIC_MAX_VOLUME || config->BufferSize > MIC_MAX_SAMPLES) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config = *config;
    hmic->AudioBuffer.Size = config->BufferSize;
    hmic->AudioBuffer.Position = 0;
    hmic->AudioBuffer.IsFull = false;

    return MIC_HW_SetSampleRate(hmic, (uint32_t)config->SampleRate);
}

MIC_StatusTypeDef MIC_SetVolume(MIC_HandleTypeDef *hmic, uint8_t volume)
{
    if (hmic == NULL || volume > MIC_MAX_VOLUME) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config.Volume = volume;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_GetVolume(const MIC_HandleTypeDef *hmic, uint8_t *volume)
{
    if (hmic == NULL || volume == NULL) {
        return MIC_INVALID_PARAM;
    }

    *volume = hmic->Config.Volume;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_SetGain(MIC_HandleTypeDef *hmic, MIC_GainTypeDef gain)
{
    if (hmic == NULL || gain > MIC_GAIN_24DB) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config.Gain = gain;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_ConfigureNoiseGate(MIC_HandleTypeDef *hmic, bool enable, float threshold)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config.NoiseGateEnable = enable;
    hmic->Config.NoiseGateThreshold = threshold;

    return MIC_OK;
}

uint32_t MIC_GetSampleRate(const MIC_HandleTypeDef *hmic)
{
    return (hmic != NULL) ? (uint32_t)hmic->Config.SampleRate : 0U;
}

MIC_StatusTypeDef MIC_GetAudioLevel(const MIC_HandleTypeDef *hmic, float *rms_level, float *peak_level)
{
    if (hmic == NULL || rms_level == NULL || peak_level == NULL) {
        return MIC_INVALID_PARAM;
    }

    *rms_level = hmic->Statistics.RMSLevel;
    *peak_level = hmic->Statistics.PeakLevel;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_GetStatistics(const MIC_HandleTypeDef *hmic, MIC_StatisticsTypeDef *stats)
{
    if (hmic == NULL || stats == NULL) {
        return MIC_INVALID_PARAM;
    }

    *stats = hmic->Statistics;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_ResetStatistics(MIC_HandleTypeDef *hmic)
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    memset(&hmic->Statistics, 0, sizeof(MIC_StatisticsTypeDef));

    return MIC_OK;
}

MIC_StatusTypeDef MIC_RegisterCallbacks(MIC_HandleTypeDef *hmic,
                                        void (*record_callback)(void),
                                        void (*error_callback)(void))
{
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    hmic->RecordCallback = record_callback;
    hmic->ErrorCallback = error_callback;

    return MIC_OK;
}
