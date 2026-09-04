/**
 ******************************************************************************
 * @file    mic_events.c
 * @brief   Interrupt-context servicing for the PDM microphone
 ******************************************************************************
 */

#include "mic_events.h"
#include "mic_core.h"
#include "mic_pdm.h"
#include "mic_dsp.h"
#include <string.h>

/**
 * @brief Decimate the captured PDM block and apply the configured filters.
 */
static void MIC_ProcessPDMData(MIC_HandleTypeDef *hmic) {
    (void)MIC_PDMToPCM(hmic->PDMBuffer, hmic->PCMBuffer, MIC_PCM_SAMPLES);

    if (hmic->Config.HighPassFilter) {
        MIC_ApplyHighPass(hmic, hmic->PCMBuffer, MIC_PCM_SAMPLES);
    }

    if (hmic->Config.NoiseGateEnable) {
        float rms_level = MIC_CalculateRMS(hmic->PCMBuffer, MIC_PCM_SAMPLES);
        if (!MIC_IsNoiseGateOpen(hmic, rms_level)) {
            memset(hmic->PCMBuffer, 0, sizeof(hmic->PCMBuffer));
        }
    }

    hmic->BufferReady = true;
}

__weak void MIC_HalfTransferCallback(MIC_HandleTypeDef *hmic) {
    if (hmic != NULL) {
        MIC_ProcessPDMData(hmic);
    }
}

__weak void MIC_TransferCompleteCallback(MIC_HandleTypeDef *hmic) {
    if (hmic != NULL) {
        MIC_ProcessPDMData(hmic);

        if (hmic->RecordCallback != NULL) {
            hmic->RecordCallback();
        }
    }
}

__weak void MIC_ErrorCallback(MIC_HandleTypeDef *hmic) {
    if (hmic != NULL) {
        hmic->Statistics.DropoutCount++;

        if (hmic->ErrorCallback != NULL) {
            hmic->ErrorCallback();
        }
    }
}

/* The HAL only hands back its own handle, so the owning driver handle is
   looked up by the block it was brought up on. */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
    MIC_HandleTypeDef *hmic = MIC_FindByInstance(hi2s->Instance);

    if (hmic != NULL) {
        MIC_HalfTransferCallback(hmic);
    }
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
    MIC_HandleTypeDef *hmic = MIC_FindByInstance(hi2s->Instance);

    if (hmic != NULL) {
        MIC_TransferCompleteCallback(hmic);
    }
}

void MIC_DMA_IRQHandler(const DMA_Stream_TypeDef *stream) {
    MIC_HandleTypeDef *hmic = MIC_FindByDMAStream(stream);

    if (hmic != NULL) {
        HAL_DMA_IRQHandler(hmic->hdma);
    }
}

void MIC_I2S_IRQHandler(const SPI_TypeDef *instance) {
    MIC_HandleTypeDef *hmic = MIC_FindByInstance(instance);

    if (hmic != NULL) {
        HAL_I2S_IRQHandler(hmic->hi2s);
    }
}
