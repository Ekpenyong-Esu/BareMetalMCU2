/**
  ******************************************************************************
  * @file    mic_analysis.c
  * @brief   Frequency-domain analysis of the captured PCM buffer
  ******************************************************************************
  */

#include "mic_analysis.h"
#include <math.h>

/* Naive O(n^2) DFT magnitude; adequate for MIC_FFT_SIZE and avoids a CMSIS-DSP dependency */
static void MIC_PerformDFT(const int16_t *input, float *magnitude, uint32_t length)
{
    for (uint32_t i = 0; i < length / 2; i++) {
        float real = 0.0f;
        float imag = 0.0f;

        for (uint32_t j = 0; j < length; j++) {
            float angle = -MIC_FFT_SCALE_FACTOR * (float)M_PI * (float)i * (float)j / (float)length;
            real += (float)input[j] * cosf(angle);
            imag += (float)input[j] * sinf(angle);
        }

        magnitude[i] = sqrtf((real * real) + (imag * imag)) / (float)length;
    }
}

static float MIC_SpectralCentroid(const float *spectrum, uint32_t length, uint32_t sample_rate)
{
    float weighted_sum = 0.0f;
    float magnitude_sum = 0.0f;

    for (uint32_t i = 0; i < length; i++) {
        float frequency = (float)i * (float)sample_rate / (float)(2U * length);
        weighted_sum += frequency * spectrum[i];
        magnitude_sum += spectrum[i];
    }

    return (magnitude_sum > 0.0f) ? (weighted_sum / magnitude_sum) : 0.0f;
}

static float MIC_ZeroCrossingRate(const int16_t *buffer, uint32_t length)
{
    uint32_t crossings = 0;

    for (uint32_t i = 1; i < length; i++) {
        if ((buffer[i - 1] >= 0) != (buffer[i] >= 0)) {
            crossings++;
        }
    }

    return (float)crossings / (float)length;
}

MIC_StatusTypeDef MIC_PerformAudioAnalysis(MIC_HandleTypeDef *hmic, MIC_AudioAnalysisTypeDef *analysis)
{
    if (hmic == NULL || analysis == NULL) {
        return MIC_INVALID_PARAM;
    }

    if (!hmic->BufferReady) {
        return MIC_BUSY;
    }

    MIC_PerformDFT(hmic->PCMBuffer, analysis->MagnitudeSpectrum, MIC_FFT_SIZE);

    analysis->Centroid = MIC_SpectralCentroid(analysis->MagnitudeSpectrum,
                                              MIC_SPECTRAL_BINS,
                                              (uint32_t)hmic->Config.SampleRate);

    analysis->ZeroCrossingRate = MIC_ZeroCrossingRate(hmic->PCMBuffer, MIC_PCM_SAMPLES);

    return MIC_OK;
}
