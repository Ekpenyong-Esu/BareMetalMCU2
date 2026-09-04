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

/* I2S2 and I2S3 are the only I2S-capable blocks, so two slots cover every
   wiring; the registry exists because HAL callbacks only hand back the I2S
   handle, which has no room for a pointer back to us. */
#define MIC_MAX_INSTANCES 2U

static MIC_HandleTypeDef *s_registry[MIC_MAX_INSTANCES];

/* ---- Registry ------------------------------------------------------------ */

MIC_HandleTypeDef *MIC_FindByInstance(const SPI_TypeDef *instance) {
    for (uint32_t i = 0U; i < MIC_MAX_INSTANCES; i++) {
        MIC_HandleTypeDef *hmic = s_registry[i];

        if (hmic != NULL && hmic->hi2s != NULL && hmic->hi2s->Instance == instance) {
            return hmic;
        }
    }

    return NULL;
}

MIC_HandleTypeDef *MIC_FindByDMAStream(const DMA_Stream_TypeDef *stream) {
    for (uint32_t i = 0U; i < MIC_MAX_INSTANCES; i++) {
        MIC_HandleTypeDef *hmic = s_registry[i];

        if (hmic != NULL && hmic->hdma != NULL && hmic->hdma->Instance == stream) {
            return hmic;
        }
    }

    return NULL;
}

static MIC_StatusTypeDef MIC_RegisterHandle(MIC_HandleTypeDef *hmic) {
    /* Two handles on one block would both claim its interrupts. */
    if (MIC_FindByInstance(hmic->Config.i2sInstance) != NULL) {
        return MIC_BUSY;
    }

    for (uint32_t i = 0U; i < MIC_MAX_INSTANCES; i++) {
        if (s_registry[i] == NULL) {
            s_registry[i] = hmic;
            return MIC_OK;
        }
    }

    return MIC_ERROR;
}

static void MIC_UnregisterHandle(const MIC_HandleTypeDef *hmic) {
    for (uint32_t i = 0U; i < MIC_MAX_INSTANCES; i++) {
        if (s_registry[i] == hmic) {
            s_registry[i] = NULL;
        }
    }
}

/* ---- Configuration ------------------------------------------------------- */

static bool MIC_IsValidAudio(const MIC_ConfigTypeDef *config) {
    return config->Volume <= MIC_MAX_VOLUME && config->BufferSize <= MIC_MAX_SAMPLES;
}

/* The wiring is fixed at MIC_Init(); a later MIC_Configure() must not move it. */
static void MIC_KeepWiring(MIC_ConfigTypeDef *dst, const MIC_ConfigTypeDef *src) {
    dst->i2sInstance = src->i2sInstance;
    dst->clkPort = src->clkPort;
    dst->clkPin = src->clkPin;
    dst->dataPort = src->dataPort;
    dst->dataPin = src->dataPin;
    dst->alternate = src->alternate;
    dst->dmaStream = src->dmaStream;
    dst->dmaChannel = src->dmaChannel;
}

MIC_StatusTypeDef MIC_GetDefaultConfig(MIC_ConfigTypeDef *config) {
    if (config == NULL) {
        return MIC_INVALID_PARAM;
    }

    memset(config, 0, sizeof(*config));

    config->SampleRate = MIC_SAMPLE_RATE_16KHZ;
    config->Gain = MIC_GAIN_0DB;
    config->Volume = MIC_DEFAULT_VOLUME;
    config->NoiseGateEnable = false;
    config->NoiseGateThreshold = MIC_NOISE_FLOOR;
    config->HighPassFilter = true;
    config->BufferSize = MIC_PCM_SAMPLES;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_Init(MIC_HandleTypeDef *hmic, I2S_HandleTypeDef *hi2s,
                           DMA_HandleTypeDef *hdma, const MIC_ConfigTypeDef *config) {
    if (hmic == NULL || hi2s == NULL || hdma == NULL || config == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!MIC_HW_IsValidWiring(config) || !MIC_IsValidAudio(config)) {
        return MIC_INVALID_PARAM;
    }

    memset(hmic, 0, sizeof(MIC_HandleTypeDef));
    hmic->hi2s = hi2s;
    hmic->hdma = hdma;
    hmic->Config = *config;

    MIC_StatusTypeDef status = MIC_RegisterHandle(hmic);
    if (status != MIC_OK) {
        return status;
    }

    status = MIC_HW_InitGPIO(config);
    if (status != MIC_OK) {
        MIC_UnregisterHandle(hmic);
        return status;
    }

    status = MIC_HW_InitI2S(hmic);
    if (status != MIC_OK) {
        MIC_HW_DeInit(hmic);
        MIC_UnregisterHandle(hmic);
        return status;
    }

    status = MIC_HW_InitDMA(hmic);
    if (status != MIC_OK) {
        MIC_HW_DeInit(hmic);
        MIC_UnregisterHandle(hmic);
        return status;
    }

    /* Marked ready before MIC_Configure() so its guard passes */
    hmic->IsInitialized = true;

    status = MIC_Configure(hmic, config);
    if (status != MIC_OK) {
        hmic->IsInitialized = false;
        MIC_HW_DeInit(hmic);
        MIC_UnregisterHandle(hmic);
        return status;
    }

    return MIC_OK;
}

MIC_StatusTypeDef MIC_DeInit(MIC_HandleTypeDef *hmic) {
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (hmic->IsRecording) {
        (void)MIC_StopRecording(hmic);
    }

    MIC_HW_DeInit(hmic);

    hmic->IsInitialized = false;
    hmic->IsRecording = false;

    MIC_UnregisterHandle(hmic);

    return MIC_OK;
}

MIC_StatusTypeDef MIC_Configure(MIC_HandleTypeDef *hmic, const MIC_ConfigTypeDef *config) {
    MIC_StatusTypeDef status = MIC_CheckReady(hmic);
    if (status != MIC_OK) {
        return status;
    }

    if (config == NULL || !MIC_IsValidAudio(config)) {
        return MIC_INVALID_PARAM;
    }

    MIC_ConfigTypeDef applied = *config;
    MIC_KeepWiring(&applied, &hmic->Config);

    hmic->Config = applied;
    hmic->AudioBuffer.Size = config->BufferSize;
    hmic->AudioBuffer.Position = 0;
    hmic->AudioBuffer.IsFull = false;

    return MIC_HW_SetSampleRate(hmic, (uint32_t)config->SampleRate);
}

MIC_StatusTypeDef MIC_SetVolume(MIC_HandleTypeDef *hmic, uint8_t volume) {
    if (hmic == NULL || volume > MIC_MAX_VOLUME) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config.Volume = volume;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_GetVolume(const MIC_HandleTypeDef *hmic, uint8_t *volume) {
    if (hmic == NULL || volume == NULL) {
        return MIC_INVALID_PARAM;
    }

    *volume = hmic->Config.Volume;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_SetGain(MIC_HandleTypeDef *hmic, MIC_GainTypeDef gain) {
    if (hmic == NULL || gain > MIC_GAIN_24DB) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config.Gain = gain;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_ConfigureNoiseGate(MIC_HandleTypeDef *hmic, bool enable, float threshold) {
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    hmic->Config.NoiseGateEnable = enable;
    hmic->Config.NoiseGateThreshold = threshold;

    return MIC_OK;
}

uint32_t MIC_GetSampleRate(const MIC_HandleTypeDef *hmic) {
    return (hmic != NULL) ? (uint32_t)hmic->Config.SampleRate : 0U;
}

MIC_StatusTypeDef MIC_GetAudioLevel(const MIC_HandleTypeDef *hmic, float *rms_level,
                                    float *peak_level) {
    if (hmic == NULL || rms_level == NULL || peak_level == NULL) {
        return MIC_INVALID_PARAM;
    }

    *rms_level = hmic->Statistics.RMSLevel;
    *peak_level = hmic->Statistics.PeakLevel;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_GetStatistics(const MIC_HandleTypeDef *hmic, MIC_StatisticsTypeDef *stats) {
    if (hmic == NULL || stats == NULL) {
        return MIC_INVALID_PARAM;
    }

    *stats = hmic->Statistics;

    return MIC_OK;
}

MIC_StatusTypeDef MIC_ResetStatistics(MIC_HandleTypeDef *hmic) {
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    memset(&hmic->Statistics, 0, sizeof(MIC_StatisticsTypeDef));

    return MIC_OK;
}

MIC_StatusTypeDef MIC_RegisterCallbacks(MIC_HandleTypeDef *hmic, void (*record_callback)(void),
                                        void (*error_callback)(void)) {
    if (hmic == NULL) {
        return MIC_INVALID_PARAM;
    }

    hmic->RecordCallback = record_callback;
    hmic->ErrorCallback = error_callback;

    return MIC_OK;
}
