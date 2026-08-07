/**
  ******************************************************************************
  * @file    mic_dsp.c
  * @brief   Level measurement and sample-domain processing
  ******************************************************************************
  */

#include "mic_dsp.h"
#include <math.h>

/* Indexed by MIC_GainTypeDef: 0/6/12/18/24 dB */
static const float s_gainFactors[] = {1.0f, 2.0f, 4.0f, 8.0f, 16.0f};

float MIC_CalculateRMS(const int16_t *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) {
        return 0.0f;
    }

    float sum_squares = 0.0f;

    for (uint32_t i = 0; i < length; i++) {
        float sample = (float)buffer[i] / MIC_PCM_SCALE_FACTOR;
        sum_squares += sample * sample;
    }

    return sqrtf(sum_squares / (float)length);
}

float MIC_CalculatePeak(const int16_t *buffer, uint32_t length)
{
    if (buffer == NULL || length == 0) {
        return 0.0f;
    }

    int16_t peak = 0;

    for (uint32_t i = 0; i < length; i++) {
        int16_t abs_sample = (buffer[i] >= 0) ? buffer[i] : (int16_t)-buffer[i];
        if (abs_sample > peak) {
            peak = abs_sample;
        }
    }

    return (float)peak / MIC_PCM_SCALE_FACTOR;
}

void MIC_ApplyScale(int16_t *buffer, uint32_t length, float factor)
{
    if (buffer == NULL) {
        return;
    }

    for (uint32_t i = 0; i < length; i++) {
        int32_t sample = (int32_t)((float)buffer[i] * factor);

        if (sample > MIC_PCM_MAX_VALUE) {
            sample = MIC_PCM_MAX_VALUE;
        } else if (sample < MIC_PCM_MIN_VALUE) {
            sample = MIC_PCM_MIN_VALUE;
        }

        buffer[i] = (int16_t)sample;
    }
}

float MIC_GainFactor(MIC_GainTypeDef gain)
{
    if ((uint32_t)gain >= (sizeof(s_gainFactors) / sizeof(s_gainFactors[0]))) {
        return MIC_GAIN_DEFAULT;
    }

    return s_gainFactors[gain];
}

void MIC_ApplyHighPass(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length)
{
    if (hmic == NULL || buffer == NULL) {
        return;
    }

    /* State lives in the handle, so it is per-instance and reset on every start */
    for (uint32_t i = 0; i < length; i++) {
        int16_t current_input = buffer[i];
        int16_t filtered = (int16_t)(((float)current_input - (float)hmic->HpPrevInput +
                                      (float)hmic->HpPrevOutput) * MIC_HP_FILTER_COEFF);
        hmic->HpPrevInput = current_input;
        hmic->HpPrevOutput = filtered;
        buffer[i] = filtered;
    }
}

bool MIC_IsNoiseGateOpen(const MIC_HandleTypeDef *hmic, float level)
{
    if (hmic == NULL) {
        return true;
    }

    float threshold_linear = powf(MIC_LOG_BASE, hmic->Config.NoiseGateThreshold / MIC_DB_SCALE_FACTOR);
    return level > threshold_linear;
}

void MIC_UpdateStatistics(MIC_HandleTypeDef *hmic, const int16_t *buffer, uint32_t length)
{
    if (hmic == NULL || buffer == NULL) {
        return;
    }

    float rms = MIC_CalculateRMS(buffer, length);
    float peak = MIC_CalculatePeak(buffer, length);

    hmic->Statistics.RMSLevel = rms;
    hmic->Statistics.PeakLevel = peak;
    hmic->Statistics.SampleCount += length;
    hmic->Statistics.AverageLevel = (1.0f - MIC_STATISTICS_ALPHA) * hmic->Statistics.AverageLevel +
                                    MIC_STATISTICS_ALPHA * rms;

    if (rms > 0.0f) {
        hmic->Statistics.SNR = MIC_DB_SCALE_FACTOR * log10f(rms / MIC_NOISE_FLOOR_LINEAR);
    }
}
