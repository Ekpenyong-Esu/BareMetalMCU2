/**
 ******************************************************************************
 * @file    mic_dsp.h
 * @brief   Level measurement and sample-domain processing
 ******************************************************************************
 */

#ifndef MIC_DSP_H
#define MIC_DSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mic_types.h"

/**
 * @brief Root-mean-square level, normalised to full scale.
 */
float MIC_CalculateRMS(const int16_t *buffer, uint32_t length);

/**
 * @brief Absolute peak level, normalised to full scale.
 */
float MIC_CalculatePeak(const int16_t *buffer, uint32_t length);

/**
 * @brief Scale a buffer in place with saturation to the 16-bit range.
 */
void MIC_ApplyScale(int16_t *buffer, uint32_t length, float factor);

/**
 * @brief Linear multiplier for a gain step.
 */
float MIC_GainFactor(MIC_GainTypeDef gain);

/**
 * @brief Single-pole high-pass, using the filter state stored in the handle.
 */
void MIC_ApplyHighPass(MIC_HandleTypeDef *hmic, int16_t *buffer, uint32_t length);

/**
 * @brief Compare a level against the configured noise-gate threshold.
 */
bool MIC_IsNoiseGateOpen(const MIC_HandleTypeDef *hmic, float level);

/**
 * @brief Refresh RMS/peak/average/SNR from the supplied buffer.
 */
void MIC_UpdateStatistics(MIC_HandleTypeDef *hmic, const int16_t *buffer, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* MIC_DSP_H */
